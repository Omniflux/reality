/**
 \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "realitypanel.h"

#include <boost/any.hpp>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSharedPointer>
#include <QShortcut>
#include <QSignalMapper>
#include <QSortFilterProxyModel>
#include <QJson/Parser>

#include "ReActionMgr.h"
#include "ReIPCCommands.h"
#include "ReMaterialPreview.h"
#include "ReSceneDataModel.h"
#include "ReTools.h"
#include "ReUiContainer.h"
#include "actions/ReMaterialActions.h"
#include "actions/ReUpdateHostTextureAction.h"
#include "exporters/json/ReJSONMaterialExporterFactory.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "exporters/lux/ReLuxMaterialExporterFactory.h"
#include "exporters/lux/ReVolumeExporter.h"
#include "exporters/qt/ReQtMaterialExporterFactory.h"
#include "exporters/qt/ReVolumeExporter.h"
#include "importers/qt/ReQtMaterialImporterFactory.h"
#include "RealityUI/ReAcselSave.h"
#include "RealityUI/ReAcselShaderSetSelector.h"
#include "RealityUI/ReExportProgressDialog.h"
#include "RealityUI/ReLightEditor.h"
#include "RealityUI/ReUniShaderSave.h"
#include "RealityUI/ReUniversalShaderSelector.h"
#include "RealityUI/MaterialEditors/ReClothEditor.h"
#include "RealityUI/MaterialEditors/ReGlassEditor.h"
#include "RealityUI/MaterialEditors/ReGlossyEditor.h"
#include "RealityUI/MaterialEditors/ReMatteEditor.h"
#include "RealityUI/MaterialEditors/ReMetalEditor.h"
#include "RealityUI/MaterialEditors/ReMirrorEditor.h"
#include "RealityUI/MaterialEditors/ReSkinEditor.h"
#include "RealityUI/MaterialEditors/ReVelvetEditor.h"
#include "RealityUI/MaterialEditors/ReWaterEditor.h"
#include "ui_reSyncMats.h"


#define RE_MAT_PREVIEW_ID "matPreview"


// Constructor: RealityPanel
RealityPanel::RealityPanel( QWidget *parent,                            
                            QString ipaddress) : 
  QWidget(parent),
  container(dynamic_cast<ReUiContainer*>(parent))
{
  setupUi(this);
  show();

  exportProgressDlg = NULL;
  currentMaterial   = NULL;
  currentObject     = NULL;
  currentMatEditor  = NULL;
  
  serverAddress = ipaddress;
  pauseMaterialPreviewGlobally(false);
  pauseMaterialPreview(false);

  updatingUI = true;

  // Add the Light Editor to the Lights tab
  reLightEditor = new ReLightEditor(swLightEditor);
  lyLightEditor->addWidget(reLightEditor);

  this->init();
  connectToPreviewMaker();

  // Create the data model for updating the host-side every time we
  // update a texture. This is set to a null texture and then set 
  // to the current texture in editTexture()
  textureChannelDataModel = QSharedPointer<ReTextureChannelDataModel>(
    new ReTextureChannelDataModel(ReTexturePtr())
  );
  connect(textureChannelDataModel.data(), 
          SIGNAL(textureUpdated(ReTexturePtr, 
                                const QString&, 
                                const QVariant&)), 
          this, SLOT(updateServerTexture(ReTexturePtr, 
                                         const QString&, 
                                         const QVariant&)));

  updatingUI = false;
}

void RealityPanel::disconnectSignals() {
  pingTimer->disconnect();
  matPreviewTimer->disconnect();
  tvMaterials->disconnect();
  textureEditor->disconnect();
  meModifiers->disconnect();
  meVolumes->disconnect();
  meMatVolumes->disconnect();
  meAlphaChannel->disconnect();
  outputOptions->disconnect();
  realityDataRelay->disconnect();
  reLightEditor->disconnect();
}

// This is called by the MainWindow when it closes. It's also called by
// the destructor of this object if it was not invoked before.
void RealityPanel::disconnect() {
  disconnectSignals();
  pingTimer->stop();
  pauseMaterialPreview(true);
  ReMaterialPreview::getInstance()->quit();
  // Wait until the preview thread ends
  previewMaker->wait(1000);

  realityDataRelay->sendMessageToServer(GUI_DISCONNECTED);
  realityDataRelay->stop();
  if (!realityDataRelay->isRunning()) {
    delete realityDataRelay;
    realityDataRelay = NULL;
  }
}

// Destructor: ~RealityPanel
RealityPanel::~RealityPanel() {
  if (realityDataRelay) {
    disconnect();
  }
  delete previewMaker;
  delete sortedSceneDataModel;
  delete sceneDataModel;
}

void RealityPanel::loadPreviewShapes() {
  bool ok = false;
  QJson::Parser json;
  QFile res(":/textResources/previewShapes.json");
  if ( res.open(QIODevice::ReadOnly) ) {
    previewShapes = json.parse(res.readAll(), &ok).toMap();
  }
  if (!ok) {
    RE_LOG_WARN() << "Error: cannot load the preview shape configuration";
  }
}

//! Set up the UI and the main events. It also starts the connection to the 
//! remote data server, the side of Reality that runs inside the hosting application.
void RealityPanel::init() {
  
  /*************************************************
   * TEMPORARILY HIDDEN BECAUSE OF ISSUES WITH POSER
   *************************************************/
   btnSave->hide();

   // The refresh feature is only for Reality for Poser
   if (RealityBase::getRealityBase()->getHostAppID() != Poser) {
     btnRefresh->hide();
   }
  /*************************************************/

  RealitySceneData->setGUIMode(true);
  // Restore the last tab active based on the save configuration from the previous run
  ReConfigurationPtr settings = RealityBase::getConfiguration();
  settings->beginGroup("MainWindow");
  int lastTab = settings->value("lastTab",0).toInt();
  settings->endGroup();
  mainTabs->setCurrentIndex(lastTab);
  setTextureEditorVisibility();

  tvMaterials->header()->setResizeMode(QHeaderView::ResizeToContents);
  // Enable the animation of the tree expansion/contraction
  tvMaterials->setAnimated(true);

  // Hide the "Rendering preview" message
  lblRenderingPreview->setText("");

  // Restore the material splitter state if available
  ReConfigurationPtr cfg = RealityBase::getRealityBase()->getConfiguration();
  if (cfg->contains( RE_CFG_MATERIAL_SPLITTER)) {
    QByteArray spData = cfg->value(RE_CFG_MATERIAL_SPLITTER).toByteArray();
    materialPageSplitter->restoreState(spData);
  }

  // Get notified whenever the use right clicks on a material
  tvMaterials->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tvMaterials,SIGNAL(customContextMenuRequested (const QPoint&)),
          this, SLOT(materialsMenuRequested(const QPoint&)));

  // Get notified whenever a camera is selected
  connect(cameraEditor, SIGNAL(cameraSelected(const QString)), 
          this, SLOT(setSelectedCamera(const QString)));

  // Start the communication channel with the server
  realityDataRelay = new RealityDataRelay(
    serverAddress, 
    RealityBase::getRealityBase()->getHostAppID());

  /*************************************************************************
   Connect the server communication system with the event handlers
   This is where the process starts. The fetchData() method is called once the connection
   with the host-app is established.
   *************************************************************************/   
  // The acknowledgment that the channel is established starts the fetching
  // of the data from the server
  connect(realityDataRelay, SIGNAL(upAndRunning()),this, SLOT(fetchData()));

  /*************************************************************************/


  // The following connections are to respond to signals from the server in case the
  // user adds, renames and deleted objects.

  // Object added
  connect(realityDataRelay, SIGNAL(objectAdded(const QString)),
          this,             SLOT(objectAdded(const QString)));

  // Object deleted
  connect(realityDataRelay, SIGNAL(objectDeleted(const QString)),
          this,             SLOT(objectDeleted(const QString)));

  // Object renamed
  connect(realityDataRelay, SIGNAL(objectRenamed(const QString, const QString)),
          this,             SLOT(objectRenamed(const QString, const QString)));
  // The object has changed internal name
  connect(realityDataRelay, SIGNAL(objectIDRenamed(const QString, const QString)),
          this,             SLOT(objectIDRenamed(const QString, const QString)));

  // The object has changed visibility
  connect(realityDataRelay, SIGNAL(objectVisibilityChanged(const QString, const bool)),
          this,             SLOT(objectVisibilityChanged(const QString, const bool)));

  // Material updated in the host-app
  connect(realityDataRelay, SIGNAL(materialUpdated(const QString, const QString)),
          this,             SLOT(materialUpdated(const QString, const QString)));

  // Material type changed
  connect(realityDataRelay, SIGNAL(materialTypeChanged(const QString, const QString, const quint16)),
          this,             SLOT(materialTypeChanged(const QString, const QString, const quint16)));

  // A material has been selected in the host-app
  connect(realityDataRelay, SIGNAL(materialSelected(const QString&, const QString&)),
          this,             SLOT(syncMaterialSelection(const QString&, const QString&)));

  // Host name available
  connect(realityDataRelay, SIGNAL(hostAppIDReady()),
          this,             SLOT(setWindowTitle()));

  // The scene has been erased. This slot needs DirectConnetion in order
  // to work correctly. There are some timing issues with the combination
  // of ZMQ + Qt signals that can cause wrong order of delivery for signals
  connect(realityDataRelay, SIGNAL(sceneHasBeenReset()),
          this,             SLOT(setNewScene()),
          Qt::DirectConnection);

  // The frame size has been changed
  connect(realityDataRelay, SIGNAL(frameSizeChanged()), 
          outputOptions, SLOT(showFrameSize()));

  // Time to close the app. Signal to close to the parent of this
  // widget
  connect(realityDataRelay, SIGNAL(close()), parent(), SLOT(close()));

  // The export was started by the host-app
  connect(realityDataRelay, SIGNAL(exportStarted(int)),
          this, SLOT(exportStarted(int)));

  // The export was started by the host-app
  connect(realityDataRelay, SIGNAL(exportingObject(const QString)),
          this, SLOT(exportingObject(const QString)));

  // The export is complete
  connect(realityDataRelay, SIGNAL(exportFinished()),
          this, SLOT(exportFinished()));

  // The render dimensions are not set
  connect(realityDataRelay, SIGNAL(renderDimensionsNotSet()),
          this, SLOT(renderDimensionsNotSet()));

  // A scene has been loaded
  connect(realityDataRelay, SIGNAL(sceneLoaded()), this, SLOT(requestDataFromHostApp()));

  // Pause/Resume the material preview
  connect(realityDataRelay, SIGNAL(pauseMaterialPreview(bool)), 
          this, SLOT(pauseMaterialPreviewGlobally(bool)));

  //! A mesh light has been converted back to its original material
  //! \param #1 objectID
  //! \param #2 materialName
  //! \param #3 lightID
  connect(reLightEditor, SIGNAL(convertToMaterial(const QString, const QString, const QString)),
          this,          SLOT(lightToMaterial(const QString, const QString, const QString)));


  // The splitter in the material editor has been moved. Save the state
  connect(materialPageSplitter, SIGNAL(splitterMoved(int,int)), 
          this, SLOT(saveSplitteState(int,int)));

  // Now that all the connections are in place, let's start the communication with the server...
  realityDataRelay->start();

  /*
   * Texture Editor
   */
  // Recursive editing of textures from the Texture Editor
  connect(textureEditor, SIGNAL(editTexture( ReTexturePtr, QString&, bool )), 
          this,          SLOT(editTexture( ReTexturePtr, QString&, bool )));

  // use of the "back" button in the breadcrumbs
  connect(textureEditor, SIGNAL(previousBreadcrumb( ReTexturePtr )), 
          this,          SLOT(previousBreadcrumb( ReTexturePtr )));

  // When the texture editor converts the type of a texture it signals that
  // we need to refresh the material view. This connection takes care of that
  // task.
  connect(textureEditor, SIGNAL(refreshMaterialView()), 
          this,          SLOT(refreshMaterialEditor()));

  // When the texture editor converts the type of a texture it signals that
  // we need to update the host-app side as well.
  // Parameters:
  //   - A string that indicates the texture name
  //   - The new type of the texture
  connect(textureEditor, SIGNAL(textureTypeHasChanged(const QString&, const ReTextureType)), 
          this,          SLOT(updateServerTextureType(const QString&, const ReTextureType)));

  // When a texture editor adds a new texture it signals that
  // we need to update the host-app side as well.
  connect(textureEditor, SIGNAL(makeNewTexture( const QString&, 
                                                const QString&, 
                                                const QString&,
                                                const QString&,
                                                const QString&,
                                                const ReTextureType ,
                                                const ReTexture::ReTextureDataType )), 
          this,          SLOT(makeNewSubTexture( const QString&, 
                                                 const QString&, 
                                                 const QString&,
                                                 const QString&,
                                                 const QString&,
                                                 const ReTextureType ,
                                                 const ReTexture::ReTextureDataType )));

  //! When a texture editor links a sub-texture to a master texture it signals that
  //! we need to update the host-app side as well.
  //! Parameters:
  //!   - objectID
  //!   - materialName
  //!   - channelName
  //!   - masterTexture
  //!   - subTexture
  connect(textureEditor, SIGNAL(linkTexture( const QString&, 
                                             const QString&, 
                                             const QString&,
                                             const QString&,
                                             const QString& )), 
          this,          SLOT(linkSubTexture( const QString&, 
                                              const QString&, 
                                              const QString&,
                                              const QString&,
                                              const QString& )));

  //! When a texture editor links a sub-texture to a master texture it signals that
  //! we need to update the host-app side as well.
  //! Parameters:
  //!   - channelID
  //!   - jsonTextureData
  //!   - masterTexture
  connect(textureEditor, SIGNAL(replaceTexture(const QString&, 
                                               const QString&,
                                               const QString& )), 
          this,          SLOT(replaceHostTexture(const QString&, 
                                                 const QString&,
                                                 const QString& )));

  //! When the texture editor handles the copy/paste operation for a texture
  //! we need to forward 
  //! we need to update the host-app side as well.
  //! Parameters:
  //!   - objectID
  //!   - materialName
  //!   - masterTexture
  //!   - channelName
  connect(textureEditor, SIGNAL(unlinkTexture( const QString&, 
                                               const QString&, 
                                               const QString&,
                                               const QString& )), 
          this,          SLOT(unlinkSubTexture( const QString&, 
                                                const QString&, 
                                                const QString&,
                                                const QString& )));

  connect(textureEditor, SIGNAL(updateModifiers()), 
          meModifiers,   SLOT(refreshUI()));

  // Editing of the textures in the Modifiers tab
  connect(meModifiers, SIGNAL(editTexture( ReTexturePtr, QString& )), 
          this,        SLOT(editTexture( ReTexturePtr, QString& )));
  // Send modifications of the parameters to the host-app side
  connect(meModifiers, SIGNAL(materialUpdated(QString, QString, QString, QVariant )), 
          this,   SLOT(updateServerMaterial( QString, QString, QString , QVariant )));
  // Send a newly created texture for a material to the host-app side
  // The parameters are: objectID, materialID, channelname, textureType, textureDataType
  connect(meModifiers, SIGNAL(makeNewTexture(const QString&, 
                                             const QString&, 
                                             const QString&, 
                                             const ReTextureType,
                                             const ReTexture::ReTextureDataType)), 
          this,     SLOT(makeNewTexture(const QString&, 
                                        const QString&, 
                                        const QString&, 
                                        const ReTextureType,
                                        const ReTexture::ReTextureDataType)));
  // Link an existing texture to a material's channel. We intercept this signal
  // so that we can notify the host-side of the change.
  // The parameters are: objectID, materialID, channelname, textureName  
  connect(meModifiers, SIGNAL(linkTexture(const QString&, const QString&, const QString&, const QString&)), 
          this,        SLOT(linkTexture(const QString&, const QString&, const QString&, const QString&)));
  // UnLink an existing texture to a material's channel. We intercept this signal
  // so that we can notify the host-side of the change.
  // The parameters are: objectID, materialID, channelname  
  connect(meModifiers, SIGNAL(unlinkTexture(const QString&, const QString&, const QString&)), 
          this,        SLOT(unlinkTexture(const QString&, const QString&, const QString&)));

  connect(meModifiers, SIGNAL(replaceTexture(const QString&, 
                                             const QString& )), 
          this,        SLOT(replaceHostTexture(const QString&, 
                                               const QString& )));

  /*
   * Alpha channel editor
   */
  // Editing of the textures in the Modifiers tab
  connect(meAlphaChannel, SIGNAL(editTexture( ReTexturePtr, QString& )), 
          this,        SLOT(editTexture( ReTexturePtr, QString& )));
  // Send modifications of the parameters to the host-app side
  connect(meAlphaChannel, SIGNAL(materialUpdated(QString , QString, QString, QVariant )), 
          this,   SLOT(updateServerMaterial( QString , QString, QString , QVariant )));
  // Send a newly created texture for a material to the host-app side
  // The parameters are: objectID, materialID, channelname, textureType, textureDataType
  connect(meAlphaChannel, SIGNAL(makeNewTexture(const QString&, 
                                             const QString&, 
                                             const QString&, 
                                             const ReTextureType,
                                             const ReTexture::ReTextureDataType)), 
          this,     SLOT(makeNewTexture(const QString&, 
                                        const QString&, 
                                        const QString&, 
                                        const ReTextureType,
                                        const ReTexture::ReTextureDataType)));
  // Link an existing texture to a material's channel. We intercept this signal
  // so that we can notify the host-side of the change.
  // The parameters are: objectID, materialID, channelname, textureName  
  connect(meAlphaChannel, SIGNAL(linkTexture(const QString&, const QString&, const QString&, const QString&)), 
          this,        SLOT(linkTexture(const QString&, const QString&, const QString&, const QString&)));
  // UnLink an existing texture to a material's channel. We intercept this signal
  // so that we can notify the host-side of the change.
  // The parameters are: objectID, materialID, channelname  
  connect(meAlphaChannel, SIGNAL(unlinkTexture(const QString&, const QString&, const QString&)), 
          this,        SLOT(unlinkTexture(const QString&, const QString&, const QString&)));

  connect(meAlphaChannel, SIGNAL(replaceTexture(const QString&, 
                                                const QString& )), 
          this,          SLOT(replaceHostTexture(const QString&, 
                                                 const QString& )));

  /*
   * Material Preview
   */
  // Material preview refresh button
  connect(btnMatPreview, SIGNAL(clicked()), this, SLOT(refreshMatPreview()));
  connect(ptCube, SIGNAL(toggled(bool)), this, SLOT(refreshMatPreview()));
  connect(ptSphere, SIGNAL(toggled(bool)), this, SLOT(refreshMatPreview()));
  connect(ptShape, SIGNAL(toggled(bool)), this, SLOT(refreshMatPreview()));
  connect(ptPlane, SIGNAL(toggled(bool)), this, SLOT(refreshMatPreview()));

  // Volumes
  connect(meVolumes, SIGNAL(volumeAdded(const QString&)), this, SLOT(volumeAdded(const QString&)));
  connect(meVolumes, SIGNAL(volumeDeleted(const QString&)), this, SLOT(volumeDeleted(const QString&)));
  connect(meVolumes, SIGNAL(volumeRenamed(const QString&, const QString&)), this, SLOT(volumeRenamed(const QString&, const QString&)));
  connect(meVolumes, SIGNAL(volumeUpdated(const QString&)), this, SLOT(volumeUpdated(const QString&)));

  // Material's volume settings
  connect(meMatVolumes, SIGNAL(materialUpdated(QString , QString, QString, QVariant )), 
          this,         SLOT(updateServerMaterial( QString , QString, QString , QVariant )));

  textureEditor->showHelpPage();

  // Render button
  connect(outputOptions, SIGNAL(renderFrame()), this, SLOT(renderFrame()));

  // Render animation button
  connect(outputOptions, SIGNAL(renderAnimation(int, int)), this, SLOT(renderAnimation(int, int)));

  // The Output Tab events
  connect(outputOptions, SIGNAL(outputDataChanged()), this, SLOT(syncSceneData()));

  // Keep track of the last tab selected by the user, we re-select it the next time
  // Reality launches
  connect(mainTabs, SIGNAL(currentChanged(int)), this, SLOT(saveMainTabsConfig(int)));

  // Initialize the Advanced Tab  
  initAdvancedTab();

  // Save the host-app scene
  connect(btnSave, SIGNAL(clicked()), this, SLOT(saveScene()));

  // Refresh the content
  connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refreshScene()));


  // Expand/Collapse all materials
  connect(btnExpandAll, SIGNAL(clicked()), this, SLOT(matExpandAll()));
  connect(btnCollapseAll, SIGNAL(clicked()), this, SLOT(matCollapseAll()));

  setMaterialContextMenu();

  /************
   * Shortcuts
   ************/
  auto renderSC = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this);
  connect(renderSC, SIGNAL(activated()), outputOptions, SIGNAL(renderFrame()));

  /************/

  // Set the timer to ping the host-app so that the host-app side of Reality
  // knows that the GUI program is running.
  pingTimer = new QTimer(this);
  connect(pingTimer, SIGNAL(timeout()), this, SLOT(pingHost()));
  pingTimer->start(1000);

  // Start the material preview timer
  matPreviewTimer = new QTimer(this);
  matPreviewTimer->setSingleShot(true);
  connect(matPreviewTimer, SIGNAL(timeout()), this, SLOT(scheduledMatPreviewHandler()));

  loadPreviewShapes();
  // Disable the material editor if there are no objects in the scene.
  materialEditor->setEnabled(false);  
}

void RealityPanel::setMaterialContextMenu() {
  auto actionMgr = ReActionManager::getInstance();

  matContexMenu          = new QMenu(this);
  toClothAction          = actionMgr->newAction("to_cloth", tr("To Cloth"), this);
  toGlassAction          = actionMgr->newAction("to_glass", tr("To Glass"), this);
  toGlossyAction         = actionMgr->newAction("to_glossy", tr("To Glossy"), this);
  toMatteAction          = actionMgr->newAction("to_matte", tr("To Matte"), this);
  toMetalAction          = actionMgr->newAction("to_metal", tr("To Metal"), this);
  toMirrorAction         = actionMgr->newAction("to_mirror", tr("To Mirror"), this);
  toNullAction           = actionMgr->newAction("to_null", tr("To Null"), this);
  toSkinAction           = actionMgr->newAction("to_skin", tr("To Skin"), this);
  toVelvetAction         = actionMgr->newAction("to_velvet", tr("To Velvet"), this);
  toWaterAction          = actionMgr->newAction("to_water", tr("To Water"), this);
  toLightAction          = actionMgr->newAction("to_light", tr("Convert to Light"), this);
  acselSaveAction        = actionMgr->newAction("save_auto_preset", tr("Save as Automatic preset"), this);
  acselDeleteShaderAction= actionMgr->newAction("rem_from_preset", tr("Remove from Automatic preset"), this);
  uShaderSaveAction      = actionMgr->newAction("save_uni_preset", tr("Save as Universal preset"), this);
  uShaderApplyAction     = actionMgr->newAction("apply_uni_preset", tr("Apply a Universal preset"), this);
  syncMatsAction         = actionMgr->newAction("sync_mats", tr("Sync"), this);
  revertToOriginalAction = actionMgr->newAction("revert_to_original", tr("Revert to original"), this);

  matContexMenu->addAction(toClothAction);
  matContexMenu->addAction(toGlassAction);
  matContexMenu->addAction(toGlossyAction);
  matContexMenu->addAction(toMatteAction);
  matContexMenu->addAction(toMetalAction);
  matContexMenu->addAction(toMirrorAction);
  matContexMenu->addAction(toNullAction);
  matContexMenu->addAction(toSkinAction);
  matContexMenu->addAction(toVelvetAction);
  matContexMenu->addAction(toWaterAction);
  matContexMenu->addSeparator();
  matContexMenu->addAction(toLightAction);
  matContexMenu->addSeparator();
  matContexMenu->addAction(acselSaveAction);
  matContexMenu->addAction(acselDeleteShaderAction);
  matContexMenu->addAction(syncMatsAction);
  matContexMenu->addSeparator();
  matContexMenu->addAction(uShaderApplyAction);
  matContexMenu->addAction(uShaderSaveAction);
  matContexMenu->addSeparator();
  matContexMenu->addAction(revertToOriginalAction);
  matContexMenu->addSeparator();

  auto copyAction = actionMgr->getAction("copy");
  auto pasteAction = actionMgr->getAction("paste");
  auto pasteWithTexturesAction = actionMgr->getAction("paste_with_textures");
  matContexMenu->addAction(copyAction);
  matContexMenu->addAction(pasteAction);
  matContexMenu->addAction(pasteWithTexturesAction);

  QSignalMapper* menuMapper = new QSignalMapper(this);
  // Assign each menu option to the corresponding material type
  // The type will be passed by the SIGNAL when triggered
  menuMapper->setMapping(toClothAction,  (int) MatCloth);
  menuMapper->setMapping(toGlassAction,  (int) MatGlass);
  menuMapper->setMapping(toGlossyAction, (int) MatGlossy);
  menuMapper->setMapping(toMatteAction,  (int) MatMatte);
  menuMapper->setMapping(toMetalAction,  (int) MatMetal);
  menuMapper->setMapping(toMirrorAction, (int) MatMirror);
  menuMapper->setMapping(toNullAction,   (int) MatNull);
  menuMapper->setMapping(toSkinAction,   (int) MatSkin);
  menuMapper->setMapping(toVelvetAction, (int) MatVelvet);
  menuMapper->setMapping(toWaterAction,  (int) MatWater);  
  menuMapper->setMapping(toLightAction,  (int) MatLight);

  connect(toClothAction,  SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toGlassAction,  SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toGlossyAction, SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toMatteAction,  SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toMetalAction,  SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toMirrorAction, SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toNullAction,   SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toSkinAction,   SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toVelvetAction, SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toWaterAction,  SIGNAL(triggered()), menuMapper, SLOT(map()));
  connect(toLightAction,  SIGNAL(triggered()), menuMapper, SLOT(map()));

  connect(menuMapper, SIGNAL(mapped(int)), this, SLOT(changeMaterialType(int)));  
  // Save the selected shaders to the ACSEL database
  connect(acselSaveAction, SIGNAL(triggered()), this, SLOT(saveACSELShaders()));

  // Delete the selected shaders from the ACSEL preset
  connect(acselDeleteShaderAction, SIGNAL(triggered()), 
          this, SLOT(deleteACSELShaders()));

  // Save the Universal shader
  connect(uShaderSaveAction, SIGNAL(triggered()), 
          this, SLOT(saveUniversalShader()));
  // Apply a Universal shader
  connect(uShaderApplyAction, SIGNAL(triggered()), 
          this, SLOT(applyUniversalShader()));
  connect(revertToOriginalAction, SIGNAL(triggered()), 
          this, SLOT(revertShaderToOriginal()));

  // Copy and paste actions
  connect(copyAction,  SIGNAL(triggered()), this, SLOT(copyMaterial()));
  connect(pasteAction, SIGNAL(triggered()), this, SLOT(pasteMaterial()));
  connect(pasteWithTexturesAction, SIGNAL(triggered()), this, SLOT(pasteMaterialWithTextures()));
  // Save menu
  connect(actionMgr->getAction("save"), SIGNAL(triggered()), 
          this, SLOT(saveScene()));

  // Apply compatible shader 
  connect(actionMgr->getAction("apply_compatible_shader_set"), SIGNAL(triggered()), 
          this, SLOT(acselApplyCompatibleShaderSet()));


  // Add the actions to the menu of the main window
  if (!container) {
    return;
  }
  container->addConvertActionToMenu(toClothAction);
  container->addConvertActionToMenu(toGlassAction);
  container->addConvertActionToMenu(toGlossyAction);
  container->addConvertActionToMenu(toMatteAction);
  container->addConvertActionToMenu(toMetalAction);
  container->addConvertActionToMenu(toMirrorAction);
  container->addConvertActionToMenu(toNullAction);
  container->addConvertActionToMenu(toSkinAction);
  container->addConvertActionToMenu(toVelvetAction);
  container->addConvertActionToMenu(toWaterAction);
  container->addConvertActionToMenu(toLightAction);
  container->addConvertActionToMenu(acselSaveAction);
  container->addConvertActionToMenu(uShaderSaveAction);
}

void RealityPanel::copyMaterial() {
  QModelIndex index = tvMaterials->currentIndex();
  ReMaterial* copyMat = sceneDataModel->getSelectedMaterial(
                            sortedSceneDataModel->mapToSource(index)
                          );
  if (copyMat) {
    auto matExporter = ReJSONMaterialExporterFactory::getExporter(copyMat);
    boost::any result;
    matExporter->exportMaterial(copyMat,result);
    QString matDef = boost::any_cast<QString>(result);
    qApp->clipboard()->setText(
      QString("%1 %2").arg(RE_CLIPBOARD_ID).arg(matDef)
    );
  }
}

void RealityPanel::pasteMaterialWithTextures() {
  pasteMaterial( ReQtMaterialImporter::Replace );
}

QList<QModelIndex> RealityPanel::checkMaterialsBeforeUpdate( ReMaterialType matType ) {
  // Find what materials are selected for the conversion
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  // Pre-scan the list to verify that only materials belonging to a single
  // object are selected
  int numRows = rows.count();
  QStringList objs;
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (mat) {
      QString objName = mat->getParent()->getName();
      if (!objs.contains(objName)) {
        objs << objName;
      }
      if (mat->getType() != matType) {
        QMessageBox::information(
          this, 
          tr("Information"), 
          tr("Only materials of the same type can be updated")
        );
        rows.clear();
        return rows;
      }
    }
  }
  if (objs.count() == 0) {
    QMessageBox::information(
      this, 
      tr("Information"), 
      tr("To use this feature one of more materials must be selected")
    );
    rows.clear();
    return rows;
  }
  if (objs.count() > 1) {
    QMessageBox::information(
      this, 
      tr("Information"), 
      tr("Only materials from one single object can be converted with"
         " this function")
    );
    rows.clear();
    return rows;
  }
  return rows;
}

void RealityPanel::pasteMaterial( ReQtMaterialImporter::ReplaceTextureMode texMode ) {

  QClipboard *clipboard = qApp->clipboard();
  // Check if the data is actually ours
  bool itsOurData = clipboard->text().startsWith(RE_CLIPBOARD_ID);
  if (!itsOurData) {
    return;
  }
  // Get the JSON data minus the signature at the beginning
  QString matStrData = clipboard->text().mid(strlen(RE_CLIPBOARD_ID)+1);
  QJson::Parser parser;
  bool ok;

  QVariantMap matData = parser.parse(matStrData.toUtf8(), &ok).toMap();
  if (!ok) {
    return;
  }
  ReMaterialType matType = ReMaterial::typeFromName(matData.value("type").toString());

  auto indices = checkMaterialsBeforeUpdate(matType);

  for (int i = 0; i < indices.count(); i++) {    
    QModelIndex index = indices[i];
    ReMaterial* theMat = sceneDataModel->getSelectedMaterial(
                              sortedSceneDataModel->mapToSource(index)
                            );
    ReMaterialPtr targetMat = RealitySceneData->getMaterial(
                                theMat->getParent()->getInternalName(),
                                theMat->getName()
                              );

    if (targetMat.isNull()) {
      return;
    }
    auto matImporter = ReQtMaterialImporterFactory::getImporter(matType);
    matImporter->importFromClipboard(targetMat, matData, texMode);
    targetMat->setEdited();
    materialSelected(index);
    // Update the material in the host-app
    QVariantMap args;
    args["objectID"]        = targetMat->getParent()->getInternalName();
    args["materialName"]    = targetMat->getName();
    args["data"]            = matStrData;
    args["replaceTextures"] = texMode;
    realityDataRelay->sendMessageToServer(MATERIAL_IMPORT_FROMCLIPBOARD_DATA, &args);
  }
}

void RealityPanel::connectToPreviewMaker() {
  // Retrieve the pointer to the material preview maker
  previewMaker = ReMaterialPreview::getInstance();
  if (previewMaker) {
    connect(previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
            this, SLOT(updatePreview(QString, QString,QImage*)));
  }  
}

void RealityPanel::pingHost() {
  realityDataRelay->sendMessageToServer(PING);
}


void RealityPanel::fetchData() {
  // Connect the method that responds to the signal that the Volumes from the 
  // host-side scene have been received and can be displayed
  connect(realityDataRelay, SIGNAL(volumesTransferred()), this, SLOT(loadVolumes()));

  // Announce to the host-side plugin that we are ready
  realityDataRelay->sendMessageToServer(GUI_CONNECTED);
  
  // Create the data model to retrieve the data from the server
  sceneDataModel = new ReSceneDataModel(this);

  // Connect the data model to the ZMQ socket class that communicates with the host-side
  sceneDataModel->setSceneData(realityDataRelay);
  
  // We want our data sorted by clicking on the header of the TreeView...
  sortedSceneDataModel = new QSortFilterProxyModel(this);
  // ... so we use a proxy and set the data model for that proxy and...
  sortedSceneDataModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  sortedSceneDataModel->setSourceModel(sceneDataModel);

  // ... give the proxy data model to our TreeView. Happiness!
  tvMaterials->setModel(sortedSceneDataModel);
  tvMaterials->model()->sort(0, Qt::AscendingOrder);

  // Connect to the selection model. This is necessary if we want to respond
  // to keyboard navigation events for the TreeView. The connection must be 
  // done after the model is set.
  QItemSelectionModel* selectModel = tvMaterials->selectionModel();
  connect(selectModel, SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
          this, SLOT(materialSelected(const QModelIndex&)));

  // Enforces to have an object or materials always selected.
  connect(selectModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          this, SLOT(checkObjectSelection( const QItemSelection&, const QItemSelection&)));

  // Forces re-selection of the first item when the data model has been reset
  connect(sceneDataModel, SIGNAL(modelReset()), this, SLOT(selectFirstItem()));

  // Reselect a material after it has been converted to a new type
  // connect(sceneDataModel, SIGNAL(materialTypeChanged(const QModelIndex&)),
  //         this, SLOT(reselectMaterial(const QModelIndex&)));
  connect(sceneDataModel, 
          SIGNAL(materialTypeChanged(const QString&, const QString&)),
          this,
          SLOT(syncMaterialSelection(const QString&, const QString&)));

  // Forward material visibility change to the host-app side.
  connect(sceneDataModel, SIGNAL(materialUpdated(QString, QString, QString, QVariant )), 
          this, SLOT(updateServerMaterial( QString, QString, QString, QVariant )));

  // 
  // LIGHTS
  // 

  // Connect the method that responds to the <LIGHTS_READY> message
  connect(realityDataRelay, SIGNAL(lightsReady()), reLightEditor, SLOT(loadLightData()));

  // If all lights have been removed then we need to reload then whole light catalog
  connect(realityDataRelay, SIGNAL(allLightsRemoved()), reLightEditor, SLOT(loadLightData()));

  // Triggered when the user changes the light type in the host-app
  connect(realityDataRelay, SIGNAL(lightTypeChanged(const QString)), 
          this, SLOT(loadLights()));

  // Light added. This is a signal received from the host-app when a new light has
  // been added to the scene by the user.
  connect(realityDataRelay, SIGNAL(lightAdded(const QString)), 
          this, SLOT(lightAdded(const QString)));

  // Signal received from the host-app that the light has been removed from the scene
  connect(realityDataRelay, SIGNAL(lightDeleted(const QString)), 
          reLightEditor, SLOT(lightDeleted(const QString)));

  // Signal received from the host-app that the light has been renamed
  connect(realityDataRelay, SIGNAL(lightRenamed(const QString, const QString)), 
          this, SLOT(lightRenamed(const QString, const QString)));

  // Signal received from the host-app that the light has been updated
  connect(realityDataRelay, SIGNAL(lightUpdated(const QString)), 
          reLightEditor, SLOT(updateLight(const QString)));

  // Signal emitted by the light editor when the user edits the light. The data is then
  // sent to the host app to update the light database.
  connect(reLightEditor, SIGNAL(lightChanged(const QString,const QString,const QVariant)), 
          this, SLOT(updateServerLight(const QString, const QString, const QVariant)));


  // Connect the method that responds to the <CAMERAS_READY> message
  connect(realityDataRelay, SIGNAL(camerasReady()), cameraEditor, SLOT(loadCameraData()));
  // Camera added
  connect(realityDataRelay, SIGNAL(cameraAdded(const QString)), this, SLOT(cameraAdded(const QString)));
  // Camera removed
  connect(realityDataRelay, SIGNAL(cameraRemoved(const QString)), this, SLOT(cameraRemoved(const QString)));

  // Camera data changed
  connect(realityDataRelay, SIGNAL(cameraChanged(const QString)), 
          cameraEditor, SLOT(cameraChanged(const QString)));
  // camera renamed
  connect(realityDataRelay, SIGNAL(cameraRenamed(const QString, const QString)),
          this,             SLOT(cameraRenamed(const QString, const QString)));

  // The active camera in the host has changed
  connect(realityDataRelay, SIGNAL(hostCameraSelected(const QString)),
          cameraEditor, SLOT(syncCameraSelectionWithHost(const QString)));
  
  // Signal emitted by the camera editor when the user edits the camera. The data is then
  // sent to the host app to update the camera database.
  connect(cameraEditor, SIGNAL(cameraChanged(const QString,const QString,const QVariant)), 
          this, SLOT(updateServerCamera(const QString, const QString, const QVariant)));

  requestDataFromHostApp();
};

void RealityPanel::requestDataFromHostApp() {
  connect(realityDataRelay, SIGNAL(outputDataReady()), this, SLOT(showOutputData()));
  // Request the scene data from the host-app side
  realityDataRelay->sendMessageToServer(GET_OUTPUT_DATA);

  // Send the message to retrieve the objects in the scene
  realityDataRelay->sendMessageToServer(GET_OBJECTS);

  // Retrieve the list of volumes
  realityDataRelay->sendMessageToServer(GET_VOLUMES);

  // Request the list of Lights to the server
  // realityDataRelay->sendMessageToServer(GET_LIGHTS);
  loadLights();

  // Request the list of cameras to the server
  realityDataRelay->sendMessageToServer(GET_CAMERAS);

  // Retrieves the library paths set by the host-app
  realityDataRelay->sendMessageToServer(GET_LIBRARY_PATHS);
}

//! Called when a figure/object is added in the host app scene while
//! Reality is running
void RealityPanel::objectAdded(const QString objectID) {
  sceneDataModel->addObject(objectID);
  meVolumes->loadData();
  // We need to re-enable the editor here because it could have been 
  // disabled by objectDeleted()
  materialEditor->setEnabled(true);  
}

//! Called when a figure/object is removed from the host app scene while
//! Reality is running
void RealityPanel::objectDeleted(const QString objName) {
  bool reloadLights = false;
  ReGeometryObjectPtr obj = RealitySceneData->getObject(objName);
  bool sameAsSelected = currentObject == obj.data();
  if (!obj.isNull()) {
    reloadLights = obj->getLights()->count() > 0;
  }
  sceneDataModel->deleteObject(objName);
  if (reloadLights) {
    reLightEditor->loadLightData();
  }
  if (sameAsSelected) {
    textureEditor->showHelpPage();
    matPages->setCurrentWidget(pageEmpty);
    currentObject = NULL;
    currentMaterial = NULL;
  }
  // Notify the Volumes editor to refresh itself
  qApp->postEvent(meVolumes, new ReVolumeListUpdatedEvent());
  materialEditor->setEnabled(RealitySceneData->getNumObjects() > 0);
}

void RealityPanel::objectRenamed(const QString objectID, const QString newName) {
  RealitySceneData->renameObject(objectID, newName);
  sceneDataModel->setNewScene();
}

void RealityPanel::objectIDRenamed(const QString objectID, const QString newID) {
  RealitySceneData->renameObjectID(objectID, newID);
  sortedSceneDataModel->invalidate();
}

void RealityPanel::objectVisibilityChanged( const QString objectID, 
                                            const bool isVisible ) 
{
  RealitySceneData->setObjectVisible(objectID, isVisible);
  if (currentObject && currentObject->getInternalName() == objectID) {
    oiIsVisible->setText(currentObject->isVisible() ? "Yes" : "No");
  }
};


void RealityPanel::loadLights() {
  // Request the list of Lights to the server
  realityDataRelay->sendMessageToServer(GET_LIGHTS);  
}

//! Called when a light is added in the host app scene while
//! Reality is running. 
void RealityPanel::lightAdded(const QString lightID) {
  reLightEditor->loadLightData();
}

//! Called when a light is renamed in the host app scene while
//! Reality is running. 
void RealityPanel::lightRenamed(const QString lightID, const QString newName) {
  RealitySceneData->renameLight(lightID, newName);
  reLightEditor->loadLightData();
}

//! Called when a camera is added in the host app scene while
//! Reality is running. We simply re-download all the cameras
void RealityPanel::cameraAdded(const QString cameraID) {
  // Request the list of cameras to the server
  realityDataRelay->sendMessageToServer(GET_CAMERAS);
}

void RealityPanel::cameraRemoved(const QString cameraID) {
  RealitySceneData->removeCamera(cameraID);
  cameraEditor->loadCameraData();
}

//! Called when a camera is renamed in the host app scene while
//! Reality is running
void RealityPanel::cameraRenamed(const QString objectID, const QString newName) {
  RealitySceneData->renameCamera(objectID, newName);
  cameraEditor->loadCameraData();
}

void  RealityPanel::setSelectedCamera(const QString cameraID) {
  QVariantMap args;
  args["value"] = cameraID;
  realityDataRelay->sendMessageToServer(UI_CAMERA_SELECTED, &args);
}

void RealityPanel::setNewScene() {
  pauseMaterialPreview(true);  
  tvMaterials->selectionModel()->clearSelection();
  sceneDataModel->setNewScene();
  // sortedSceneDataModel->invalidate();
  currentMaterial = NULL;
  currentObject = NULL;

  reLightEditor->eraseAllLights();

  RealitySceneData->newScene();
  RealitySceneData->setGUIMode();
  reLightEditor->loadLightData();
  outputOptions->updateUI();
  currentMaterial = NULL;
  matPreview->setPixmap(QPixmap(QString::fromUtf8(":/images/matPreviewBlank.png")));
  pauseMaterialPreview(false);

  selectFirstItem();  
}

void RealityPanel::loadVolumes() {
  meVolumes->loadData();
}

void RealityPanel::materialTypeChanged( const QString objectID, 
                                        const QString materialName, 
                                        const quint16 newTypeInt ) 
{
  pauseMaterialPreview(true);
  
  tvMaterials->selectionModel()->clearSelection();
  ReMaterialType newType = static_cast<ReMaterialType>(newTypeInt);

  // Since the materials have been updated we need to refresh the volumes as well
  realityDataRelay->sendMessageToServer(GET_VOLUMES);
  currentMaterial = NULL;
  if (newType == MatLight) {
    sceneDataModel->updateMaterial(objectID, materialName, true);
    sceneDataModel->refresh();
    loadLights();
  }
  else {
    sceneDataModel->updateMaterial(objectID, materialName, false);
  }
  pauseMaterialPreview(false);
}

//! Called when a material is updated in the host app scene while
//! Reality is running
void RealityPanel::materialUpdated( const QString materialName, 
                                    const QString objectID ) 
{
  pauseMaterialPreview(true);
  
  ReMaterialPtr mat = RealitySceneData->getObject(objectID)->getMaterial(materialName);
  if (mat.isNull()) {
    RE_LOG_DEBUG() << QSS(QString("Error: material %1 for object %2 is NULL")
                            .arg(materialName)
                            .arg(objectID));
    return;
  }
  // Since the materials have been updated we need to refresh the volumes as well
  realityDataRelay->sendMessageToServer(GET_VOLUMES);

  QString objectName = RealitySceneData->getObject(objectID)->getName();
/*
  bool userModified = mat->isEdited();
  if (userModified) {
    if ( QMessageBox::question( this, 
                                "Material Modified", 
                                QString(tr("Material %1 of %2 has been modified in the scene but it was also modified"
                                        " in Reality. Do you want to overwrite your changes with the data from" 
                                        " the scene?"))
                                  .arg(materialName).arg(objectName),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No
                              ) == QMessageBox::No ) {
      return;
    }
  }
  */
  currentMaterial = NULL;
  // Needed because the material will be deleted 
  currentIndex = tvMaterials->currentIndex();
  tvMaterials->selectionModel()->clearSelection();

  sceneDataModel->updateMaterial(objectID, materialName);
  QAbstractItemModel* model = tvMaterials->model();
  QModelIndexList items = model->match(
                            model->index(0, 0),
                            Qt::DisplayRole,
                            QVariant::fromValue(materialName),
                            2,
                            Qt::MatchRecursive); 
  if (items.count() > 0) {
    tvMaterials->selectionModel()->select(
      items[0], 
      QItemSelectionModel::Select | QItemSelectionModel::Rows | QItemSelectionModel::Current
    );
    model->sort(0, Qt::AscendingOrder);
    materialSelected(items[0]);
  }
  pauseMaterialPreview(false);
  refreshMatPreview();
}

void RealityPanel::lightToMaterial( const QString objectID,
                                    const QString materialName, 
                                    const QString lightID ) 
{
  pauseMaterialPreview(true);
  currentMaterial = NULL;
  tvMaterials->selectionModel()->clearSelection();
  ReMaterialPtr lightMat = RealitySceneData->getMaterial(
                                  objectID, 
                                  materialName
                           );
  if (lightMat->getType() != MatLight) {
    QMessageBox::information(
      this,
      tr("Information"),
      QString(
        tr("Error: the material is not a light! It's a %1")
      ).arg(lightMat->getTypeAsString())
    );
    return;
  }
  QVariantMap args;
  args["objectID"] = objectID;
  args["matID"]    = materialName;
  args["newType"]  = lightMat.staticCast<ReLightMaterial>()->getOriginalType();
  realityDataRelay->sendMessageToServer(CHANGE_MATERIAL_TYPE, &args);
  RealitySceneData->deleteLight(lightID);
  reLightEditor->loadLightData();
  pauseMaterialPreview(false);
}

void RealityPanel::syncMaterialSelection(const QString& objectID, const QString& matID) {
  ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
  if (obj.isNull()) {
    return;
  }
  QAbstractItemModel* model = tvMaterials->model();
  QModelIndexList items = model->match(
              model->index(0, 0),
              Qt::DisplayRole,
              QVariant::fromValue(obj->getName()),
              1,
              Qt::MatchExactly); 
  if (items.count() > 0) {
    // Select just the object
    if (matID == "") {
      tvMaterials->selectionModel()->select(
        items[0], 
        QItemSelectionModel::Select  | 
        QItemSelectionModel::Rows    | 
        QItemSelectionModel::Current
      );            
      tvMaterials->scrollTo(items[0], 
                            static_cast<QAbstractItemView::ScrollHint>(
                              QAbstractItemView::EnsureVisible | 
                              QAbstractItemView::PositionAtCenter
                            ));
      materialSelected(items[0]);
      return;
    }
    // Select the material
    QModelIndexList matIndexes = model->match(
              items[0],
              Qt::DisplayRole,
              QVariant::fromValue(matID),
              1,
              Qt::MatchRecursive);
    if (matIndexes.count() > 0) {
      tvMaterials->selectionModel()->select(
        matIndexes[0], 
        QItemSelectionModel::Select | QItemSelectionModel::Rows | QItemSelectionModel::Current
      );
      tvMaterials->scrollTo(matIndexes[0], 
                            static_cast<QAbstractItemView::ScrollHint>(
                              QAbstractItemView::EnsureVisible | 
                              QAbstractItemView::PositionAtCenter
                            ));
      materialSelected(matIndexes[0]);
    }
  }
  sortedSceneDataModel->sort(0);
}


void RealityPanel::setWindowTitle() {
  if (!container) {
    return;
  }
  container->setTitle(
    QString(tr("Reality %1 Edition"))
      .arg(RealityBase::getRealityBase()->getPrintableHostAppID())
  );
}


void RealityPanel::saveSplitteState(int, int) {
  ReConfigurationPtr cfg = RealityBase::getRealityBase()->getConfiguration();
  cfg->setValue(RE_CFG_MATERIAL_SPLITTER, materialPageSplitter->saveState());
}

/**
 Method: editTexture

 This is where the editing of textures starts. The method simply dispatches the command to the
 Texture Editor if a couple of basic conditions are verified: 
    - The current material is valid and
    - The texture to be edited is not null
 */
void RealityPanel::editTexture( ReTexturePtr tex, 
                                QString& breadcrumb, 
                                const bool isRoot ) {
  if (!currentMaterial) {
    return;
  }
  if (!tex.isNull()) {
    if (textureChannelDataModel.isNull()) {
      RE_LOG_INFO() << "TextureChannel data model is NULL";
      return;
    }
    textureChannelDataModel->setTexture(tex);
    textureEditor->setModel(textureChannelDataModel, breadcrumb, isRoot);
  }
}


/*
 * We receive the signal from the breadcrumb widget here that the user
 * clicked the "back" button once. The parameter is the pointer to 
 * the texture that needs to become current.
 */
void RealityPanel::previousBreadcrumb( ReTexturePtr tex ) {
  QString dummy;
  editTexture( tex, dummy, false );
}

void RealityPanel::updateServerTexture( ReTexturePtr texture, 
                                        const QString& paramName, 
                                        const QVariant& value ) 
{
  ReMaterial* mat = const_cast<ReMaterial*>(
                      static_cast<const ReMaterial*>(texture->getParent())
                    );
  // We find the channel so that we can identify the textures in the same channel
  // when the user is using multi-edit. For example, if the user is editing
  // the Diffuse textures for multiple-materials, we want to apply the same 
  // transformation to all selected materials.
  QString channelID = mat->identifyChannel(texture);

  QVariantMap args;
  // Check if we have multiple materials selected. In that case
  // we are going to update the same value for all of them.
  // This is the multi-edit feature for textures
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();
  for (int i = 0; i < numRows; i++) {
    // Obtain the material selected
    ReMaterial* mat2 = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                       );
    // Find the texture affected inside the selected material. This identifies
    // the texture for what it is in relation to the material, independently from
    // the actual texture that was edited.

    auto tex2 = mat2->findTextureByChannelPath(channelID);

    // In the unlikely case that the texture cannot be identifies we need to stop
    // right away
    if (tex2.isNull()) {
      continue;
    }
    // If the type of texture differs from the texture used to do the edit then
    // we cannot apply the change and we need to bail out
    if (tex2->getType() != texture->getType()) {
      continue;
    }
    // Apply the transformation to the texture in the client app
    tex2->setNamedValue(paramName, value);
    // Apply the same transformation to the host-side copy of the texture
    ReUpdateHostTextureAction act(mat2->getParent()->getInternalName(),
                                  mat2->getName(),
                                  paramName,
                                  tex2,
                                  value);
    act.execute();
    mat2->setEdited();
    // RE_LOG_INFO() << "Multi-edited material " << mat2->getName().toStdString()
    //               << ", " << channelID.toStdString()
    //               << ", " << tex2->getName().toStdString()
    //               << ", " << paramName.toStdString()
    //               << ", " << value.toString().toStdString();
  }
  refreshMaterial();
}


void RealityPanel::makeNewTexture( const QString& objectID,
                                   const QString& materialID,
                                   const QString& channelName,
                                   const ReTextureType textureType,
                                   const ReTexture::ReTextureDataType dtype ) {

  QVariantMap args;

  // Check if we have multiple materials selected. In that case
  // we are going to update the same value for all of them.
  // This is the multi-edit feature
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();
  auto srcMat = RealitySceneData->getObject(objectID)->getMaterial(materialID);
  // RE_LOG_INFO() << "Multi-creating " << numRows << " textures";
  for (int i = 0; i < numRows; i++) {

    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                        sortedSceneDataModel->mapToSource(rows[i])
                      );
    // The source texture has been already created in the GUI. We need
    // to do the same for the other textures selected
    if (mat != srcMat) {
      // If the makeNeTexture method returns false then the operation 
      // failed and we need to avoid replicating the texture in the
      // host side.
      if ( !mat->makeNewTexture( channelName, textureType, dtype ) ) {
        continue;
      }
    }
    mat->setEdited();
    // Prepare the data to be sent to the server
    // args["objectID"]     = objectID;
    args["objectID"]     = mat->getParent()->getInternalName();
    // args["materialName"] = materialID;
    args["materialName"] = mat->getName();
    args["channelName"]  = channelName;
    args["textureType"]  = textureType;
    args["dataType"]     = dtype;
    realityDataRelay->sendMessageToServer(MAKE_NEW_TEXTURE, &args);
  }
  refreshMaterial();
}


void RealityPanel::makeNewSubTexture( const QString& objectID, 
                                      const QString& materialName, 
                                      const QString& channelName,
                                      const QString& masterTextureName,
                                      const QString& newTextureName,
                                      const ReTextureType texType,
                                      const ReTexture::ReTextureDataType texDataType ) {
  QVariantMap args;
  // Prepare the data to be sent to the server
  args["objectID"]          = objectID;
  args["materialName"]      = materialName;
  args["channelName"]       = channelName;
  args["masterTextureName"] = masterTextureName;
  args["newTextureName"]    = newTextureName;
  args["textureType"]       = texType;
  args["dataType"]          = texDataType;
  realityDataRelay->sendMessageToServer(MAKE_NEW_SUB_TEXTURE, &args);
  refreshMaterial();  
}

void RealityPanel::linkTexture( const QString& objectID,
                                const QString& materialID,
                                const QString& channelName,
                                const QString& textureName ) {

  QVariantMap args;
  // Prepare the data to be sent to the server
  args["objectID"]     = objectID;
  args["materialName"] = materialID;
  args["channelName"]  = channelName;
  args["textureName"]  = textureName;
  realityDataRelay->sendMessageToServer(LINK_TEXTURE, &args);
  refreshMaterial();
}

void RealityPanel::linkSubTexture( const QString& objectID,
                                   const QString& materialID,
                                   const QString& channelName,
                                   const QString& textureName,
                                   const QString& subTextureName ) {

  QVariantMap args;
  // Prepare the data to be sent to the server
  args["objectID"]       = objectID;
  args["materialName"]   = materialID;
  args["channelName"]    = channelName;
  args["textureName"]    = textureName;
  args["subTextureName"] = subTextureName;
  realityDataRelay->sendMessageToServer(LINK_SUB_TEXTURE, &args);
  refreshMaterial();
}

void RealityPanel::unlinkSubTexture( const QString& objectID,
                                     const QString& materialID,
                                     const QString& textureName,
                                     const QString& channelName )
{
  QVariantMap args;
  // Prepare the data to be sent to the server
  args["objectID"]       = objectID;
  args["materialName"]   = materialID;
  args["textureName"]    = textureName;
  args["channelName"]    = channelName;
  realityDataRelay->sendMessageToServer(UNLINK_SUB_TEXTURE, &args);
  refreshMaterial();
}

void RealityPanel::unlinkTexture( const QString& objectID,
                                  const QString& materialID,
                                  const QString& channelName ) {

  #ifndef NDEBUG
  RE_LOG_INFO() << "Deleting texture: " << QString("%1, %2, %3")
                                             .arg(objectID)
                                             .arg(materialID)
                                             .arg(channelName);
  #endif
  QVariantMap args;
  // Prepare the data to be sent to the server
  args["objectID"]     = objectID;
  args["materialName"] = materialID;
  args["channelName"]  = channelName;
  realityDataRelay->sendMessageToServer(UNLINK_TEXTURE, &args);
  refreshMaterial();
}

void RealityPanel::replaceHostTexture( const QString& channelID,
                                       const QString& jsonTextureData,
                                       const QString& masterTextureName ) 
{
  QVariantMap args;

  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();

  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (!mat) {
      continue;
    }
    ReTexturePtr masterTexture;
    if (!masterTextureName.isEmpty()) {
      masterTexture = mat->getTexture(masterTextureName);
      if (masterTexture.isNull()) {
        continue;
      }
    }
    mat->replaceTexture(channelID, jsonTextureData, masterTexture);

    // Prepare the data to be sent to the server
    args["objectID"]     = mat->getParent()->getInternalName();
    args["materialName"] = mat->getName();
    args["channelName"]  = channelID;
    args["textureData"]  = jsonTextureData;
    args["masterTextureName"] = masterTextureName;
    realityDataRelay->sendMessageToServer(REPLACE_TEXTURE, &args);
  }

  refreshMaterial();
}

void RealityPanel::updateServerTextureType( const QString& textureName, 
                                            const ReTextureType newType) {

  auto tex = currentMaterial->getTexture(textureName);
  QString channelID = currentMaterial->identifyChannel(tex);
  // Check if we have multiple materials selected. In that case
  // we are going to update the same value for all of them.
  // This is the multi-edit feature
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (!mat) {
      RE_LOG_INFO() << "Mat #" << i << " i null";
      continue;
    }
    // auto tex2 = mat->getChannel(channelID);
    auto tex2 = mat->findTextureByChannelPath(channelID);

    if (tex2.isNull()) {
      RE_LOG_INFO() << "Mat " << mat->getName().toStdString()
                    << " does not have channel " << channelID.toStdString();
      continue;
    }

    // Apply the change to all the materials, GUI-side
    if (mat->getUniqueName() != currentMaterial->getUniqueName()) {
      mat->changeTextureType(tex2->getName(), newType);      
    }
    // Apply the change to all the materials, host-side
    QVariantMap args;
    // Prepare the data to be sent to the server
    args["objectID"]     = mat->getParent()->getInternalName();
    args["materialName"] = mat->getName();
    args["textureName"]  = tex2->getName();
    args["newType"]      = newType;
    mat->setEdited();
    realityDataRelay->sendMessageToServer(CHANGE_TEXTURE_TYPE, &args);
  }
  refreshMaterial();
}

void RealityPanel::updateServerLight( const QString lightID, 
                                      const QString paramName, 
                                      const QVariant value ) {
  QVariantMap args;
  args["lightID"]   = lightID;
  args["valueName"] = paramName;
  args["value"]     = value;
  realityDataRelay->sendMessageToServer(UI_LIGHT_EDITED, &args);
}

void RealityPanel::updateServerCamera( const QString cameraID, 
                                       const QString paramName, 
                                       const QVariant value ) {
  QVariantMap args;
  args["cameraID"]  = cameraID;
  args["valueName"] = paramName;
  args["value"]     = value;
  realityDataRelay->sendMessageToServer(UI_CAMERA_EDITED, &args);
}

// Utility function used to send a message to the host side that
// a property of a material has been updated. 
void updateHostMaterialProperty( 
                                 const QString& objectID,
                                 const QString& materialName,
                                 const QString& propName,
                                 const QVariant& value
                               )
{
  QVariantMap args;
  args["objectID"]     = objectID;
  args["materialName"] = materialName;
  args["propertyName"] = propName;
  args["value"]        = value;
  realityDataRelay->sendMessageToServer(UI_MATERIAL_EDITED, &args);
}

void RealityPanel::updateServerMaterial( QString objectID, 
                                         QString materialName, 
                                         QString valueName, 
                                         QVariant value ) {
  QVariantMap args;
  // Check if we have multiple materials selected. In that case
  // we are going to update the same value for all of them.
  // This is the multi-edit feature
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();
  if (numRows > 1) {
    for (int i = 0; i < numRows; i++) {
      ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                           sortedSceneDataModel->mapToSource(rows[i])
                        );
      if (!mat) {
        continue;
      }
      // Prepare the data to be sent to the server
      updateHostMaterialProperty(mat->getParent()->getInternalName(), 
                                 mat->getName(), 
                                 valueName, 
                                 value);
      // If we are updating more than one material then we need
      // to update the materials on the GUI side, not just the server side.
      if (mat != currentMaterial) {
        mat->setNamedValue(valueName, value);
      }
      refreshMaterial(mat, false);
    }
  }
  else {
    // Send the data to the server
    updateHostMaterialProperty(objectID, materialName, valueName, value);
  }

  refreshMaterial(currentMaterial);
}

void RealityPanel::refreshMaterial( ReMaterial* mat, bool refreshPreview ) {
  if (!mat) {
    mat = currentMaterial;
  }
  if (!mat) {
    return;
  }
  mat->setEdited();
  // refresh the UI with the new data
  matPages->currentWidget()->update();
  // Refresh the list of materials tree view to show the updated material
  tvMaterials->update();

  if (refreshPreview) {
    refreshMatPreview();
  }
}

QString RealityPanel::useShapePreview( const QString& matName, 
                                       const QString& geomName ) 
{
  if (previewShapes.contains(geomName)) {
    return previewShapes.value(geomName).toMap().value(matName,"").toString();
  }
  return "";
}

bool RealityPanel::isMaterialPreviewEnabled() {
  return !globalPreviewPaused && !localPreviewPaused;
}

void RealityPanel::pauseMaterialPreview( bool pause ) {    
  localPreviewPaused = pause;
}

void RealityPanel::pauseMaterialPreviewGlobally( bool pause ) {    
  globalPreviewPaused = pause;
  if (currentMaterial && !pause) {
    scheduleMatPreview();
  }
}

void RealityPanel::scheduleMatPreview() {
  matPreviewTimer->stop();
  matPreviewTimer->start(800);    
};

void RealityPanel::scheduledMatPreviewHandler() {
  matPreviewTimer->stop();
  refreshMatPreview(false);
}

void RealityPanel::refreshMatPreview( bool forceRefresh ) {
  if (!isMaterialPreviewEnabled() || previewMaker == NULL) {
    return;
  }

  lblRenderingPreview->setText(tr("Rendering preview..."));
  btnMatPreview->setEnabled(false);
  ReLuxTextureExporter::initializeTextureCache();
  ReLuxTextureExporter::enableTextureCache(false);
  // Get the material definition
  auto exporter = ReLuxMaterialExporterFactory::getExporter(currentMaterial);

  boost::any textureData;
  exporter->exportTextures(currentMaterial, textureData, true);
  QString matDefinition = boost::any_cast<QString>(textureData);

  boost::any matData;
  exporter->exportMaterial(currentMaterial, matData);
  matDefinition += boost::any_cast<QString>(matData);
  // Export the volumes
  QString volumeDefinition;
  ReLuxVolumeExporter volExporter;

  ReVolumeIterator vi(RealitySceneData->getVolumes());
  while( vi.hasNext() ) {
    vi.next();
    volumeDefinition += volExporter.exportVolume(vi.value());
  }

  QString previewType = useShapePreview( currentMaterial->getName(), 
                                         currentObject->getGeometryFileName() );
  if ( previewType.isEmpty() ) {
    if (!(ptCube->isChecked() || ptPlane->isChecked())) {
      ptSphere->setChecked(true);
    }
  }
  else {
    ptShape->setChecked(true);
  }

  if ( !ptShape->isChecked() ) {
    previewType = "sphere";
  }
  if (ptPlane->isChecked()) {
    previewType = "plane";
  }
  else if (ptCube->isChecked()) {
    previewType = "cube";
  }

  ReMaterialPreview::getInstance()->sendPreviewRequest(
    currentMaterial->getUniqueName(),
    QString(RE_MAT_PREVIEW_ID),
    previewType,
    volumeDefinition + matDefinition,
    false,
    forceRefresh
  );
}

void RealityPanel::updatePreview( QString matName, 
                                  QString previewID,
                                  QImage* preview ) 
{
  if (previewMaker == NULL) {
    return;
  }
  if (!currentMaterial) {
    return;
  }
  if (previewID == RE_MAT_PREVIEW_ID && 
      currentMaterial && 
      matName == currentMaterial->getUniqueName()) 
  {
    matPreview->setPixmap(QPixmap::fromImage(*preview));
    // We are responsible for the disposal of this resources
    delete preview;
  }
  lblRenderingPreview->setText("");
  btnMatPreview->setEnabled(true);
}

void RealityPanel::selectFirstItem() {
  // Select the first item in the list of objects
  QItemSelectionModel* selectionModel = tvMaterials->selectionModel();
  tvMaterials->setCurrentIndex(selectionModel->model()->index(0, 0));
}

void RealityPanel::checkObjectSelection( const QItemSelection& selected, 
                                         const QItemSelection& deselected) 
{
  int selectedItems = tvMaterials->selectionModel()->selectedRows().count();
  // If there are no items selected as the result of the user's action
  // then we select the first item in the list, which is the object's
  // record of the first object
  if (selectedItems == 0) {
    selectFirstItem();
  }
}

// Removes all the child widgets from a layout. This function is used by the 
// the \ref makeNewEditor() method to configure the material editor needed
// to edit the current material
void clearLayout(QLayout* layout) {
  while (QLayoutItem* item = layout->takeAt(0)) {
    QWidget* widget = item->widget();
    if (widget) {
      delete widget;
    }
    delete item;
  }
}

template<typename T> void RealityPanel::makeNewEditor() {
  auto newEditor = new T(this);
  clearLayout(editorScrollerLayout);
  editorScrollerLayout->addWidget(newEditor);
  // Add a vertical spacer to keep the editor aligned with the top of
  // the scroll area
  editorScrollerLayout->addItem(
    new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding)
  );

  currentMatEditor = newEditor;
  addMatEditorSignals(newEditor);
}

void RealityPanel::addMatEditorSignals( ReMaterialEditor* editor ) {
  connect(editor, SIGNAL(editTexture( ReTexturePtr, QString& )), 
          this,   SLOT(editTexture( ReTexturePtr, QString& )));

  connect(editor, SIGNAL(materialUpdated(QString , QString, QString, QVariant )), 
          this,   SLOT(updateServerMaterial( QString , QString, QString , QVariant )));
  // Send a newly created texture for a material to the host-app side
  // The parameters are: objectID, materialID, channelname, textureType, textureDataType
  connect(editor, SIGNAL(makeNewTexture(const QString&, 
                                        const QString&, 
                                        const QString&, 
                                        const ReTextureType,
                                        const ReTexture::ReTextureDataType)), 
          this,     SLOT(makeNewTexture(const QString&, 
                                        const QString&, 
                                        const QString&, 
                                        const ReTextureType,
                                        const ReTexture::ReTextureDataType)));
  // Link an existing texture to a material's channel. We intercept this signal
  // so that we can notify the host-side of the change.
  // The parameters are: objectID, materialID, channelname, textureName  
  connect(editor, SIGNAL(linkTexture(const QString&, const QString&, const QString&, const QString&)), 
          this,   SLOT(linkTexture(const QString&, const QString&, const QString&, const QString&)));
  // UnLink an existing texture to a material's channel. We intercept this signal
  // so that we can notify the host-side of the change.
  // The parameters are: objectID, materialID, channelname  
  connect(editor, SIGNAL(unlinkTexture(const QString&, const QString&, const QString&)), 
          this,   SLOT(unlinkTexture(const QString&, const QString&, const QString&)));

  connect(editor, SIGNAL(replaceTexture(const QString&, const QString&)), 
          this,   SLOT(replaceHostTexture(const QString&, const QString&)));

}

void RealityPanel::materialSelected(const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  // Enable the editor
  materialEditor->setEnabled(true);  

  bool altKeyPressed = qApp->keyboardModifiers() & Qt::AltModifier;

  currentIndex = index;
  // We retrieve the index to the data model by querying the sorted data model.
  // The *mapToSource()* method gives us the index into the real data model, the
  // one used by the sorting data model. Then we pass the index to the data model
  // to retrieve both the corresponding object and the material, if any. There is
  // no guarantee that we have a material because the use might have clicked on the
  // Object "header" record, the one that represents the object in the tree view.
  currentObject = sceneDataModel->getSelectedObject(
                                    sortedSceneDataModel->mapToSource(index)
                                  );

  currentMaterial = sceneDataModel->getSelectedMaterial(
                                      sortedSceneDataModel->mapToSource(index)
                                    );
  if (!currentMaterial) {
    // If we select a top-level object and not a material then we need to 
    // reset the Texture Editor to a neutral state
    textureEditor->showHelpPage();
    pauseMaterialPreview(true);
    showObjectInformation();
    return;    
  }

  updatingUI = true;

  // A material has been selected. First enable all the tabs of the material editor.
  // They might have been disabled when selecting an object entry.
  materialEditor->setTabEnabled(1, true);
  materialEditor->setTabEnabled(2, true);
  materialEditor->setTabEnabled(3, true);

  // As we select a new material we set the Texture Editor to a neutral state
  textureEditor->showHelpPage();
  textureEditor->resetBreadcrumbs();

  bool usesModifiers = true;
  bool usesAlpha = true;
  bool usesVolumes = false;
  ReMaterialType matType = currentMaterial->getType();

  matPages->setCurrentWidget(pageEditor);

  switch(matType) {
    case MatGlass: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReGlassEditor")) ) {
        makeNewEditor<ReGlassEditor>();
        connect(currentMatEditor, SIGNAL(volumeAdded(const QString)), 
                this, SLOT(volumeAdded(const QString&)));
        connect(currentMatEditor, SIGNAL(volumeUpdated(const QString&)), 
                this, SLOT(volumeUpdated(const QString&)));
      }
      usesVolumes = true;
      break;
    }
    case MatGlossy: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReGlossyEditor")) ) {
        makeNewEditor<ReGlossyEditor>();
      }
      usesVolumes = true;
      break;
    }
    case MatCloth: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReClothEditor")) ) {
        makeNewEditor<ReClothEditor>();
      }
      break;
    }
    case MatMatte: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReMatteEditor")) ) {
        makeNewEditor<ReMatteEditor>();
      }
      usesVolumes = true;
      break;
    }
    case MatMetal: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReMetalEditor")) ) {
        makeNewEditor<ReMetalEditor>();
      }
      break;
    }
    case MatMirror: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReMirrorEditor")) ) {
        makeNewEditor<ReMirrorEditor>();
      }
      break;
    }
    case MatNull: {
      matPages->setCurrentWidget(pageNull);
      usesVolumes = true;
      currentMatEditor = NULL;
      usesAlpha = false;
      usesModifiers = false;
      break;
    }
    case MatSkin: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReSkinEditor")) ) {
        makeNewEditor<ReSkinEditor>();
        connect(currentMatEditor, SIGNAL(materialVolumeChanged(const QString&, bool)),
                meMatVolumes, SLOT(materialVolumeChanged(const QString&, bool)));
      }
      usesVolumes = true;
      break;
    }
    case MatVelvet: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReVelvetEditor")) ) {
        makeNewEditor<ReVelvetEditor>();
      }
      break;
    }
    case MatLight: {
      textureEditor->showHelpPage();
      updatingUI = false;
      return;
    }
    case MatWater: {
      if (!currentMatEditor || 
          (currentMatEditor && !currentMatEditor->inherits("ReWaterEditor")) ) {
        makeNewEditor<ReWaterEditor>();
        connect(currentMatEditor, SIGNAL(refreshModifiers()), 
                meModifiers, SLOT(refreshUI()));
        connect(currentMatEditor, SIGNAL(refreshVolumes()), 
                this, SLOT(loadVolumes()));
        // The Water material updates directly the texture used for displacing the 
        // water material. The following connection updates the host-app side.
        connect(currentMatEditor, SIGNAL(textureUpdated(ReTexturePtr, const QString&, const QVariant&)), 
                this, SLOT(updateServerTexture(ReTexturePtr, const QString&, const QVariant&)));
      }
      usesVolumes = true;
      usesAlpha = false;
      break;
    }
    case MatUndefined: {
      currentMatEditor = NULL;
      usesModifiers = false;
      usesAlpha = false;
      matPages->setCurrentWidget(pageNull);
      QMessageBox::information(this, "Information", "The Selected material is undefined");
      break;
    }
    case MatMix:
      //!TODO!
      break;
    case MatHair:
      //! Not implemented yet
      break;
  }
  if (currentMatEditor) {
    currentMatEditor->setData(currentMaterial, textureChannelDataModel);
  }
  // Set up the modifiers
  if (usesModifiers) {
    meModifiers->setData(
      static_cast<ReModifiedMaterial*>(currentMaterial),
      textureChannelDataModel
    );
  }
  materialEditor->setTabEnabled(materialEditor->indexOf(matModifiers), usesModifiers);
  
  if (usesAlpha) {
    meAlphaChannel->setData(currentMaterial, textureChannelDataModel);
    materialEditor->setTabEnabled(materialEditor->indexOf(matAlpha), true);
  }
  else {
    materialEditor->setTabEnabled(materialEditor->indexOf(matAlpha), false);
  }

  if (usesVolumes) {
    // Enable the Volumes tab
    materialEditor->setTabEnabled(materialEditor->indexOf(matVolumes), true);
    meMatVolumes->setMaterial(currentMaterial);    
  }
  else {
    // Disable the Volumes tab
    materialEditor->setTabEnabled(materialEditor->indexOf(matVolumes), false);
  }
  pauseMaterialPreview(false);

  // Update the material preview
  scheduleMatPreview();
  updatingUI = false;

  // If the Alt/Opt key is pressed then select the material in the 
  // host application
  if (altKeyPressed) {
    QVariantMap args;
    args["objectID"] = currentObject->getInternalName();
    args["materialName"] = currentMaterial->getName();
    realityDataRelay->sendMessageToServer(SELECT_MATERIAL_IN_HOST, &args);
  }
}

void RealityPanel::refreshMaterialEditor() {
  if (currentMatEditor) {
    currentMatEditor->setData(currentMaterial, textureChannelDataModel);
  }
}

// Right-click menu for the materials
void RealityPanel::materialsMenuRequested(const QPoint& pos) {  
  // Pre-scan the list to verify that only materials belonging to a single
  // object and of the same type are selected
  auto selectionModel = tvMaterials->selectionModel();
  QList<QModelIndex> rows = selectionModel->selectedRows();
  int numRows = rows.count();
  // Exit in case, for any reason, the use right-clicked while no selection
  // is present.
  if (!numRows) {
    return;
  }

  // Keep track of how many objects, instead of materials, has been selected
  // by the user. If this number is greater than zero at the end of the loop
  // then we abort. Only materials can be used for this function.
  int numObjectsSelected = 0;
  for (int i = 0; i < numRows; i++) {
    auto mat = sceneDataModel->getSelectedMaterial(
                 sortedSceneDataModel->mapToSource(rows[i])
               );
    if (!mat) {
      numObjectsSelected++;
    }
  }
  if (numObjectsSelected > 0) {
    QMessageBox::information(
      this, 
      tr("Information"), 
      tr("Only materials can be selected. There are no functions available"
         " to be performed on whole objects.\n"
         "Please deselect the object and try again.")
    );
    return;
  }

  // Enable/disable the Universal Shaders action
  uShaderSaveAction->setEnabled(numRows == 1);

  // If we get here then we know that only materials have been selected.  
  QStringList objs;
  ReMaterial* selectedMat = sceneDataModel->getSelectedMaterial(
    sortedSceneDataModel->mapToSource(selectionModel->currentIndex())
  );

  ReMaterialType matType = selectedMat->getType();
  QString matName = selectedMat->getName();
  bool sameType = true;
  QList<ReMaterial*> mats;
  for (int i = 0; i < numRows; i++) {
    auto mat = sceneDataModel->getSelectedMaterial(
                 sortedSceneDataModel->mapToSource(rows[i])
               );
    if (mat != selectedMat) {
      mats.append(mat);
    }
    if (mat) {
      QString objName = mat->getParent()->getName();
      if (!objs.contains(objName)) {
        objs << objName;
      }
      if (mat->getType() != matType) {
        sameType = false;
      }
    }
  }
  // Disable the actions that are not usable when working with multiple
  // objects.
  if (objs.count() > 1) {
    acselSaveAction->setEnabled(false);
    acselDeleteShaderAction->setEnabled(false);
  }
  else {
    acselSaveAction->setEnabled(true);
    acselDeleteShaderAction->setEnabled(true);
  }
  // Set the default text for this menu
  syncMatsAction->setText(tr("Sync"));
  if (sameType) {
    if (matType == MatGlossy || matType == MatSkin) {
      syncMatsAction->setEnabled(true);
      syncMatsAction->setText(QString(tr("Sync with %1")).arg(matName));
    }
    else {
      syncMatsAction->setEnabled(false);
    }
  }
  else {
    syncMatsAction->setEnabled(false);
  }
  // Call the menu
  if (currentMaterial) {
    auto newPos = pos;
    // move the position of the menu away from the mouse pointer to avoid
    // activating an option by mistake
    newPos.rx() += 10;
    auto action = matContexMenu->exec(tvMaterials->mapToGlobal(newPos));
    if (action == syncMatsAction) {
      synchronizeMaterials(selectedMat, mats);
    }
  }
}

void RealityPanel::changeMaterialType( int newType ) {
  // Find what materials are selected for the conversion
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (mat) {
      RealityBase::getUndoStack()->push(
        new ChangeMaterialTypeAction( mat->getParent()->getInternalName(),
                                      mat->getName(),
                                      static_cast<ReMaterialType>(newType) ) 
      );
    }
  }
}

void RealityPanel::volumeAdded(const QString& volName) {
  QVariantMap args;
  args["volumeName"] = volName;
  realityDataRelay->sendMessageToServer(VOLUME_ADDED, &args);
  if (currentMaterial) {
    meMatVolumes->setMaterial(currentMaterial);
  }
  meVolumes->loadData();
};

void RealityPanel::volumeDeleted(const QString& volName) {
  QVariantMap args;
  args["volumeName"] = volName;
  realityDataRelay->sendMessageToServer(VOLUME_DELETED, &args);
  if (currentMaterial) {
    meMatVolumes->setMaterial(currentMaterial);
  }
  meVolumes->loadData();
};

void RealityPanel::volumeRenamed(const QString& oldName, const QString& newName) {
  QVariantMap args;
  args["oldVolumeName"] = oldName;
  args["newVolumeName"] = newName;
  realityDataRelay->sendMessageToServer(VOLUME_RENAMED, &args);
  if (currentMaterial) {
    meMatVolumes->setMaterial(currentMaterial);
  }
  meVolumes->loadData();
};

void RealityPanel::volumeUpdated(const QString& volName) {
  QVariantMap args;
  args["volumeName"] = volName;
  realityDataRelay->sendMessageToServer(VOLUME_UPDATED, &args);
  meVolumes->loadData();
};

/*
 * The Advanced Tab
 */
void RealityPanel::initAdvancedTab() {
  ReConfigurationPtr config = RealityBase::getConfiguration();
  matCacheSize->setValue(config->value(RE_CFG_MAT_PREVIEW_CACHE_SIZE).toInt());
  luxDisplayRefresh->setValue(config->value(RE_CFG_LUX_DISPLAY_REFRESH).toInt());
  luxWriteInterval->setValue(config->value(RE_CFG_LUX_WRITE_INTERVAL).toInt());
  ignoreDistantLights->setChecked(config->value(RE_CFG_IGNORE_INFINITE).toBool());
  convertDistantLights->setChecked(config->value(RE_CFG_CONVERT_INFINITE).toBool());
  convertSpotLights->setChecked(config->value(RE_CFG_USE_REAL_SPOTS).toBool());
  overwriteWarning->setChecked(config->value(RE_CFG_OVERWRITE_WARNING).toBool());
  // gpuOn->setChecked(config->value(RE_CFG_USE_GPU).toBool());
  OCLGroupSize->setValue(config->value(RE_CFG_OCL_GROUP_SIZE).toInt());
  newSceneFolder->setText(config->value(RE_CFG_DEFAULT_SCENE_LOCATION).toString());
  keepUIResponsive->setChecked(config->value(RE_CFG_KEEP_UI_RESPONSIVE).toBool());
  keepUIResponsive->setChecked(config->value(RE_CFG_KEEP_UI_RESPONSIVE).toBool());

  connect(matCacheSize, SIGNAL(valueChanged(int)), this, SLOT(updateConfiguration(int)));
  connect(luxDisplayRefresh, SIGNAL(valueChanged(int)), this, SLOT(updateConfiguration(int)));
  connect(luxWriteInterval, SIGNAL(valueChanged(int)), this, SLOT(updateConfiguration(int)));
  connect(ignoreDistantLights, SIGNAL(toggled(bool)), this, SLOT(updateConfiguration(bool)));
  connect(convertDistantLights, SIGNAL(toggled(bool)), this, SLOT(updateConfiguration(bool)));
  connect(convertSpotLights, SIGNAL(toggled(bool)), this, SLOT(updateConfiguration(bool)));
  connect(overwriteWarning, SIGNAL(toggled(bool)), this, SLOT(updateConfiguration(bool)));
  // connect(gpuOn, SIGNAL(toggled(bool)), this, SLOT(updateConfiguration(bool)));
  connect(OCLGroupSize, SIGNAL(valueChanged(int)), this, SLOT(updateConfiguration(int)));
  connect(newSceneFolder, SIGNAL(textChanged(const QString&)), this, SLOT(updateDefaultLocation(const QString&)));
  connect(btnNewSceneFolder, SIGNAL(clicked()), this, SLOT(browseDefaultFolder()));
  connect(keepUIResponsive, SIGNAL(toggled(bool)), this, SLOT(updateConfiguration(bool)));
}

void RealityPanel::browseDefaultFolder() {
  QString newDir = QFileDialog::getExistingDirectory(
    this,
    tr("Select a folder where to store newly created scenes"),
    newSceneFolder->text()
  );
  if (newDir != "") {
    newSceneFolder->setText(newDir);
    ReConfigurationPtr config = RealityBase::getConfiguration();
    config->setValue(RE_CFG_DEFAULT_SCENE_LOCATION, newDir);
    config->sync();
  }
}

void RealityPanel::updateDefaultLocation(const QString& newDir) {
  newSceneFolder->setText(newDir);
  ReConfigurationPtr config = RealityBase::getConfiguration();
  config->setValue(RE_CFG_DEFAULT_SCENE_LOCATION, newDir);
  config->sync();
}

void RealityPanel::updateConfiguration( int newVal ) {
  QString source = QObject::sender()->objectName();
  ReConfigurationPtr config = RealityBase::getConfiguration();

  if (source == "matCacheSize") {
    config->setValue(RE_CFG_MAT_PREVIEW_CACHE_SIZE, newVal);
  }
  else if (source == "luxDisplayRefresh") {
    config->setValue(RE_CFG_LUX_DISPLAY_REFRESH, newVal);
  }
  else if (source == "luxWriteInterval") {
    config->setValue(RE_CFG_LUX_WRITE_INTERVAL, newVal);
  }
  else if (source == "OCLGroupSize") {
    config->setValue(RE_CFG_OCL_GROUP_SIZE, newVal);
  }
  config->sync();
}

void RealityPanel::updateConfiguration( bool newVal ) {
  QString source = QObject::sender()->objectName();
  ReConfigurationPtr config = RealityBase::getConfiguration();

  if (source == "ignoreDistantLights") {
    config->setValue(RE_CFG_IGNORE_INFINITE, newVal);    
  }
  else if (source == "convertDistantLights") {
    config->setValue(RE_CFG_CONVERT_INFINITE, newVal);    
  }
  else if (source == "convertSpotLights") {
    config->setValue(RE_CFG_USE_REAL_SPOTS, newVal);    
  }
  else if (source == "overwriteWarning") {
    config->setValue(RE_CFG_OVERWRITE_WARNING, newVal);    
  }
  else if (source == "keepUIResponsive") {
    config->setValue(RE_CFG_KEEP_UI_RESPONSIVE, newVal);
  }

  config->sync();
}

void RealityPanel::syncSceneData() {
  realityDataRelay->sendMessageToServer(SYNC_OUTPUT_DATA);

  // If the user reconnects from Lux after the path was empty we want to
  // reconnect the material preview. This generally happens when the user
  // set the path to Lux for the first time. At launch Reality tries to 
  // find Lux automatically but if that is not possible then the path to Lux
  // is left empty and the material preview doesn't work. With the following
  // step we avoid requiring to restart Reality.
  if (!previewMaker) {
    connectToPreviewMaker();
  }
}

void RealityPanel::showOutputData() {
  outputOptions->updateUI();
}

int RealityPanel::numOfActiveLights() {
  int numLights = 0;
  // Scan the lights
  ReLightIterator i(RealitySceneData->getLights());
  while( i.hasNext() ) {
    i.next();
    if (i.value()->isLightOn()) {
      numLights++;
    }
  }
  // Search if we have any light-emitting material
  auto objs = RealitySceneData->getObjects();
  ReGeometryObjectIterator gi(objs);
  while( gi.hasNext() ) {
    gi.next();
    auto obj = gi.value();
    if (obj.isNull()) {
      continue;
    }
    ReMaterialIterator l(obj->getMaterials());
    while( l.hasNext() ) {
      l.next();
      if (l.value()->getNamedValue("emitsLight").toBool()) {
        numLights++;
      }
    }
  }
  return numLights;
}

bool RealityPanel::renderPreFlightCheck() {
  if (RealitySceneData->getNumObjects() == 0) {
    QMessageBox::information(this,
                             tr("Information"),
                             tr("There are no objects in the scene."));
    return false;
  }
  if (numOfActiveLights() == 0) {
    QMessageBox::information(this,
                             tr("Information"),
                             tr("There are no sources of light in the scene.\nPlease add or enable at least one light and try again."));
    return false;
  }
  if (RealitySceneData->getCameras()->count() == 0) {
    QMessageBox::information(
      this,
      tr("Information"),
      tr("There are no cameras in the scene.\nPlease add "
         "at least one camera and try again.")
    );
    return false;
  }
  if (RealitySceneData->getSelectedCamera().isNull()) {
    QMessageBox::information(
      this,
      tr("Information"),
      tr("No camera has been selected to render the scene.\nPlease select "
         "one camera and try again.")
    );
    return false;
  }
  return true;
}

void RealityPanel::renderFrame() {
  if (!renderPreFlightCheck()) {
    return;
  }
  // Check if the path for the files exists
  QFileInfo fInfo;
  fInfo.setFile(RealitySceneData->getImageFileName());
  if (!fInfo.absoluteDir().exists()) {
    QMessageBox::information(
      this,
      tr("Information"),
      tr("The folder for the image file does not exist.\n"
         "Please select a new destination folder."
        )
    );
    return;
  }
  fInfo.setFile(RealitySceneData->getSceneFileName());
  if (!fInfo.absoluteDir().exists()) {
    QMessageBox::information(
      this,
      tr("Information"),
      tr("The folder for the exported scene file does not exist.\n"
         "Please select a new destination folder."
        )
    );
    return;
  }
  QString luxPath = RealityBase::getRealityBase()->getRendererPath(LuxRender);
  if (luxPath.isEmpty() || !QFileInfo(luxPath).exists()) {
    QMessageBox::information(
      this, 
      "Information", 
      "LuxRender was not found in your installation. Please re-install Reality"
      " and make sure to not alter any of its files. Thank you.\n"
    );
    return;
  }
  QFileInfo sceneFile = RealitySceneData->getSceneFileName();
  bool warn = RealitySceneData->getOverwriteWarning();

  if (warn && sceneFile.exists()) {
    if ( QMessageBox::question(this, 
                               "Overwrite confirmation", 
                               QString(
                                tr("The scene file %1 already exists. Do you want to overwrite it?")
                               )
                               .arg(sceneFile.absoluteFilePath()),
                               QMessageBox::Yes | QMessageBox::No,
                               QMessageBox::No) != QMessageBox::Yes ) 
    {
      return;
    }
  }
  QVariantMap args;
  args["runRenderer"] = !outputOptions->chbExportOnly->isChecked();
  realityDataRelay->sendMessageToServer(RENDER_FRAME, &args);
}


void addHashMarks( QString& fileName, quint16 howMany ) {
  QFileInfo finfo(fileName);
  QString hashMarks;
  for (int i = 0; i < howMany; i++) {
    hashMarks += "#";
  }
  QString suffix = finfo.completeSuffix();
  fileName = QString("%1/%2_%3%4")
              .arg(finfo.path())
              .arg(finfo.baseName())
              .arg(hashMarks)
              .arg(suffix != "" ? QString(".") + suffix : "");
}

void RealityPanel::renderAnimation(int startFrame, int endFrame) {
  if (!renderPreFlightCheck()) {
    return;
  }

  int maxSamples = RealitySceneData->getMaxSPX();
  if (maxSamples == 0) {
    QMessageBox::information(
      this, 
      "Information", 
      "To render an animation you must set the maximum amount of samples per frame. "
      "See the \"Output\" panel in the Render tab."
    );
    return;
  }

  QString sceneFileName, imageFileName;
  sceneFileName = RealitySceneData->getSceneFileName();
  imageFileName = RealitySceneData->getImageFileName();

  bool needsUpdate = false;
  if (!sceneFileName.contains('#')) {
    addHashMarks(sceneFileName, 6);
    RealitySceneData->setSceneFileName(sceneFileName);
    needsUpdate = true;
  }
  if (!imageFileName.contains('#')) {
    addHashMarks(imageFileName, 6);
    RealitySceneData->setImageFileName(imageFileName);
    needsUpdate = true;
  }
  if (needsUpdate) {
    outputOptions->updateUI();
    syncSceneData();
  }
  QVariantMap args;
  args["runRenderer"] = !outputOptions->chbExportOnly->isChecked();
  args["startFrame"] = startFrame;
  args["endFrame"] = endFrame;
  realityDataRelay->sendMessageToServer(RENDER_ANIMATION, &args);
}


void RealityPanel::saveScene() {
  realityDataRelay->sendMessageToServer(SAVE_SCENE);
}

void RealityPanel::refreshScene() {
  realityDataRelay->sendMessageToServer(REFRESH_SCENE);
}

void RealityPanel::matExpandAll() {
  tvMaterials->expandAll();
}

void RealityPanel::matCollapseAll() {
  tvMaterials->collapseAll();
}

void RealityPanel::setServerAddress( const QString _serverAddress ) {
  serverAddress = _serverAddress;
}

void RealityPanel::setTextureEditorVisibility() {
  int tabNum = mainTabs->currentIndex();
  // Show hide the textures editor based on the tab selected
  bool showTextureEditor = false;
  if (tabNum == 1 ) { // Materials
    showTextureEditor = true;
  }

  gbTextureEditor->setVisible(showTextureEditor);  
}

void RealityPanel::saveMainTabsConfig( int tabNum ) {
  ReConfigurationPtr settings = RealityBase::getRealityBase()->getConfiguration();
  settings->beginGroup("MainWindow");
  settings->setValue("lastTab", tabNum);
  settings->endGroup();

  setTextureEditorVisibility();
}

void RealityPanel::showObjectInformation() {
  matPages->setCurrentWidget(pageObjectInfo);
  oiObjectName->setText(currentObject->getName());
  oiInternalName->setText(currentObject->getInternalName());
  oiGeometryFile->setText(currentObject->getGeometryFileName());
  oiIsVisible->setText(currentObject->isVisible() ? tr("Yes") : tr("No"));
  oiIsInstance->setText(currentObject->isInstance() ? tr("Yes") : tr("No"));
  oiNumMaterials->setText(QString::number(currentObject->getNumMaterials()));
  materialEditor->setTabEnabled(1, false);
  materialEditor->setTabEnabled(2, false);
  materialEditor->setTabEnabled(3, false);
}

void RealityPanel::exportStarted( int numObject ) {
  if (!exportProgressDlg) {
    exportProgressDlg = new ReExportProgressDialog(this);
  }
  exportProgressDlg->reset();
  exportProgressDlg->setUpperLimit(numObject);
  exportProgressDlg->show();
}

void RealityPanel::exportingObject(const QString msg) {
  if (!exportProgressDlg) {
    RE_LOG_WARN() << "You must call exportStarted() before calling exportingObject()";
    return;
  }
  exportProgressDlg->showMessage(msg);
  exportProgressDlg->increment();
}

void RealityPanel::exportFinished() {
  exportProgressDlg->close();
}

void RealityPanel::renderDimensionsNotSet() {
  HostAppID appID = RealityBase::getRealityBase()->getHostAppID();

  if (appID == Poser) { 
    QMessageBox::information(this, 
                             tr("Information"), 
                             tr("Please set a defined resolution for your render in Poser and "
                                " make sure that the render dimensions are set to \"Render to exact resolution\". "
                                "\n\nSee the \"Render | Render Dimensions...\" menu in Poser"));
  }
}

int RealityPanel::findShaderVolumeInAcsel( const QString& objectID,
                                           const QString& setID, 
                                           const QString volumeID ) {
  // First we find all the materials that use the volume
  QStringList matNames = RealitySceneData->getMaterialsLinkedToVolume(
    objectID, volumeID
  );
  auto acsel = ReAcsel::getInstance();
  return acsel->getVolumeID( setID, matNames );
}

bool RealityPanel::findAcselShaderSet( QString& shaderSet ) {
  shaderSet.clear();

  // Case 1: All selected materials belong to the same shader set:
  QSet<QString> setsFound;
  QMap<QString, QString> setNames;

  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (mat) {
      QString ss = mat->getAcselSetID();
      if (!ss.isEmpty() && 
           mat->getAcselSetName() != RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL) {
        setsFound << ss;
      }
    }
  }
  int numSets = setsFound.count();
  // Case 2: the user tried to save shaders belonging to different sets.
  // This is a mistake
  if ( numSets > 1) {
    QMessageBox::warning(this, 
                         tr("Error found"), 
                         tr("Cannot save materials that belong to different shader sets")
                        );
    return false;
  }
  // Case 1: all selected materials belong to an existing shader set.
  if (numSets == 1) {
    shaderSet = *setsFound.constBegin();
    return true;
  }

  // Case 3-4: some materials are assigned to a set and some of the 
  //          selected materials are not. We find the set used and 
  //          return it.

  // Scan all the materials of the object, not just the materials selected,
  // to find the shader set, if any, already in use. 
  setsFound.clear();
  ReMaterialDictionary materials = currentObject->getMaterials();
  ReMaterialIterator i(materials);
  while( i.hasNext() ) {
    i.next();
    auto mat = i.value();
    // QString ss = mat->getAcselSetName();
    QString ss = mat->getAcselSetID();
    if (!ss.isEmpty() && 
         mat->getAcselSetName() != RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL) {
      setsFound << ss;
      setNames[mat->getAcselSetName()] = ss;
    }
  }
  numSets = setsFound.count();
  if (numSets == 0) {
    shaderSet.clear();
    return true;
  }
  if (numSets == 1) {
    shaderSet = *setsFound.constBegin();
    return true;
  }
  bool ok;
  // If we get here it means that we have found multiple sets in the
  // materials and we need to ask the user which one has to be used
  QString choice = QInputDialog::getItem(
                this, 
                tr("Select the shader set"), 
                tr("Please select which shader set should be used"),
                setNames.keys(),
                0,
                false,
                &ok
              );
  shaderSet.clear();
  if (!ok) {
    return false;
  }
  // Now find the code for the selection
  shaderSet = setNames.value(choice);
  return true;
}

//! Removes all the textures from a material, except the ones that are
//! used in the channels and that are of type TexConstant. 
//! This is done to strip the generic/default shaders of the textures
//! but keep the solid colors used in channels like the Diffuse and
//! Specular one, which are often needed for setting the materials 
//! properly for the general use. This happens, for example, with the
//! glass materials used for the cornea.
void stripTextures( QVariantMap& data ) {
  QVariantList textures = data["textures"].toList();
  QVariantList newTextures;
  QSet<QString> channelTextures;

  // Create a set with the name of the textures used directly
  // by the material's channels for easy lookup
  QVariantMap channels = data["channels"].toMap();
  QMapIterator<QString, QVariant> i(channels);
  while( i.hasNext() ) {
    i.next();
    QString v = i.value().toString();
    if (!v.isEmpty()) {
      channelTextures << v;
    }
  }

  // Check if textures of type Constant are used directly by the
  // material's channels
  int numTextures = textures.count();
  for (int i = 0; i < numTextures; i++) {
    QVariantMap t = textures[i].toMap();
    if ( t["type"] == TexConstant && 
         channelTextures.contains(t["name"].toString()) ) {
      newTextures << t;
    }
  }
  // Replace the original textures with the ones obtained by the
  // previous filter
  data["textures"] = newTextures;
}

void RealityPanel::saveACSELShaders() {
  QString setID; 
  QString setName;
  // Find what materials are selected for the conversion
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();

  // Pre-scan the list to verify that only materials belonging to a single
  // object are selected
  int numRows = rows.count();

  if ( !findAcselShaderSet(setID) ) {
    return;
  }

  ReAcselSave acselSaveDlg(
    currentObject->getGeometryFileName(), setID, this
  );
  if ( !acselSaveDlg.exec() ) {
    return;
  }


  // Store the author's name in the Reality configuration so that it can be
  // used in the future
  auto cfg = RealityBase::getConfiguration();
  QString author = acselSaveDlg.getAuthor();
  cfg->setValue(RE_CFG_AUTHOR_NAME, author);
  cfg->sync();
  setID          = acselSaveDlg.getSetID();
  setName        = acselSaveDlg.getSetName();

  ReAcsel* acsel = ReAcsel::getInstance();
  bool isDefault = acselSaveDlg.isDefaultConfig();
  bool setMaterialTypeOnly = acselSaveDlg.mustSetMaterialTypeOnly();
  bool isNew     = acselSaveDlg.isNew();
  if (setName == RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL) {
    QMessageBox::information(
      this, 
      tr("Information"), 
      QString(
        tr("I'm sorry but a the shader set name %1 is not allowed")
      )
      .arg(RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL)
    );
    return;    
  }
  if (isNew) {
    QVariantMap setData;
    if (acsel->findShaderSet(setName, setData, true)) {
      QMessageBox::information(
        this, 
        tr("Information"), 
        QString(
          tr("I'm sorry but a shader set with the same name already "
             "exists for object \"%1\". Please try again with a different "
             "name"
          )
        )
        .arg(setData[RE_ACSEL_BUNDLE_OBJECT_ID].toString())
      );
      return;
    }
    // If we get to this point it means that everything is OK. We have a new
    // set name and it doesn't exist in the ACSEL database. All that we need
    // is to blank the set id so that a new ID will be returned by the 
    // saveShaderSet() method
    setID = "";
  }

  bool setSaved = acsel->saveShaderSet(
                    setID,
                    setName, 
                    acselSaveDlg.getDescription(), 
                    author,
                    currentObject->getGeometryFileName(),
                    isDefault
                  );
  if (!setSaved) {
    QMessageBox::information(
      this, tr("Error"), tr("Error while trying to save the preset")
    );
  }
  QString objectID = currentObject->getInternalName();

  // Find what materials are selected for the conversion
  rows.clear();
  rows = tvMaterials->selectionModel()->selectedRows();
  numRows = rows.count();
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (mat) {
      // Export the material to the QVariantMap format
      auto matExporter = ReQtMaterialExporterFactory::getExporter(mat);
      boost::any result;
      matExporter->exportMaterial(mat, result);
      // Convert from Boost::any to QVariantMap
      QVariantMap qvmShaderData = boost::any_cast<QVariantMap>(result);
      // If this is a generic shader then strip the textures, they are not
      // needed
      if (isDefault) {
        stripTextures(qvmShaderData);
      }
      // Set the flag if this shader needs to be used only to
      // change the material type. The flag is actually a comma-separated
      // list of the renderers that generated the shader that will be converted
      // by Reality.
      if (setMaterialTypeOnly) {
        switch(RealityBase::getRealityBase()->getHostAppID()) {
          case Poser:
            qvmShaderData["typeOnly"] = QString("%1,%2")
                                        .arg(RE_HOST_RENDERER_CODE_FIREFLY)
                                        .arg(RE_HOST_RENDERER_CODE_SUPERFLY);
            break;
          case DAZStudio:
            qvmShaderData["typeOnly"] = QString("%1,%2")
                                        .arg(RE_HOST_RENDERER_CODE_IRAY)
                                        .arg(RE_HOST_RENDERER_CODE_3DELIGHT);
            break;
          default:
            throw(std::invalid_argument(
                   QString("Host id of %1 is not valid for setting the type of the shader ")
                   .arg(RealityBase::getRealityBase()->getHostAppIDAsString())
                   .toStdString()
                 )
            );
            break;
        }
      }
      // Serialize the data to store it in the database
      QJson::Serializer json;
      QString matDef = json.serialize(qvmShaderData);
      QString matName = mat->getName();
      // If the material uses a volume, like for SSS skin, we need to
      // save that first.
      QString volumeName = mat->getInnerVolume();
      int volumeID = 0;
      if (!volumeName.isEmpty()) {
        volumeID = savePresetVolume(objectID, setID, volumeName);
      }
      // Compute the ACSEL ID based on the original data of the shader
      // because that is what will be used to find the ACSEL shader the next
      // time the material is created
      // QVariantMap origShaderData;
      QString acselID = mat->getAcselID();
      QStringList maps;

      if (isDefault) {
        acselID = acsel->getAcselID(
                     mat->getParent()->getGeometryFileName(), matName, maps
                  );
      }
      else {
        // if ( acsel->getOriginalShader(objectID, matName, origShaderData) ) {
        //   getImageMapsOrColors(origShaderData, maps);
        //   acselID = acsel->getAcselID(
        //                mat->getParent()->getGeometryFileName(), matName, maps
        //             );
        // }
        acselID = mat->computeAcselID();
      }
      if ( 
           acsel->saveShader(
             acselID, matDef, mat->getType(), matName, setID, volumeID
           ) 
         )
      {
        QString acselSetName = (isDefault ? RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL 
                                  : setName);
        mat->setAcselSetName(acselSetName);      
        mat->setAcselID(acselID);
        mat->setAcselSetID(setID);
        mat->setEdited(false);

        // Update the material in the host app as well
        updateServerMaterial(objectID,matName,"acselSetName",acselSetName);
        updateServerMaterial(objectID,matName,"acselSetID",setID);
        updateServerMaterial(objectID,matName,"acselID",acselID);
        updateServerMaterial(objectID,matName,"edited", false);
      }
    }    
    // Cause the widget to refresh so that the name of the shader is visible,
    // next to the material name
    sceneDataModel->refreshRow(sortedSceneDataModel->mapToSource(rows[i]));
  }
}

void RealityPanel::deleteACSELShaders() {
  int retval = QMessageBox::question(
                 this, 
                 tr("Confirmation needed"), 
                 tr("Are you sure that you want to delete the selected "
                    "shaders?"),
                 QMessageBox::Yes | QMessageBox::No,
                 QMessageBox::No
               );
  if( retval != QMessageBox::Yes ) {
    return;
  }
  // Find what materials are selected for the conversion
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();
  auto acsel = ReAcsel::getInstance();
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (mat) {
      bool isGeneric = mat->getAcselSetName() == RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL;
      QString shaderID;
      if (isGeneric) {
        shaderID = mat->computeAcselID(true);
      }
      else {
        shaderID = mat->getAcselID();
      }
      RE_LOG_INFO() << "Deleting shader " << shaderID.toStdString();
      if( acsel->deleteShader(shaderID) ) {
        mat->setAcselSetID("");
        mat->setAcselSetName("");
        updateServerMaterial(mat->getParent()->getInternalName(),
                             mat->getName(),
                             "acselSetID",
                             "");
        updateServerMaterial(mat->getParent()->getInternalName(),
                             mat->getName(),
                             "acselSetName",
                             "");
      }
      else {
        RE_LOG_INFO() << "  >>> Shader could not be deleted!";
      }
    }
  }
  tvMaterials->update();
}

void RealityPanel::acselApplyCompatibleShaderSet() {
  // Find what materials are selected for the conversion
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  // Pre-scan the list to verify that only materials belonging to a single
  // object are selected
  int numRows = rows.count();
  QStringList objs;
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (mat) {
      QString objName = mat->getParent()->getName();
      if (!objs.contains(objName)) {
        objs << objName;
      }
    }
  }
  if (objs.count() == 0) {
    QMessageBox::information(
      this, 
      tr("Information"), 
      tr("To use this feature one of more materials must be selected")
    );
    return;
  }
  if (objs.count() > 1) {
    QMessageBox::information(
      this, 
      tr("Information"), 
      tr("Only materials from one single object can be converted with"
         " this function")
    );
    return;
  }

  // Ask the user to select a shader set...
  ReAcselShaderSetSelector shaderSetSelector(this);
  if ( shaderSetSelector.exec() == QDialog::Rejected ) {
    return;
  }

  auto shaderSetID = shaderSetSelector.getSelectedShaderSetID();

  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    // It should never happen but you never know...
    if (!mat) {
      continue;
    }
    QString matName = mat->getName();
    QVariantMap args;
    
    args["objectID"] = mat->getParent()->getInternalName();
    args["matID"]    = matName;
    args["setID"]    = shaderSetID;
    realityDataRelay->sendMessageToServer(APPLY_ACSEL_SHADER, &args);
  }

}

void RealityPanel::synchronizeMaterials( const ReMaterial* baseMat, 
                                         QList<ReMaterial*>& toBeSynchronized )
{
  QDialog dlg(this);
  Ui::syncMats syncMatsUI;
  syncMatsUI.setupUi(&dlg);

  // Show the name of the base material in the dialog's caption
  syncMatsUI.caption->setText(
    QString(syncMatsUI.caption->text()).arg(baseMat->getName())
  );
  switch( baseMat->getType() ) {
    case MatGlossy: {
      syncMatsUI.topCoatFlag->show();
      syncMatsUI.translucencyFlag->show();
      syncMatsUI.sssFlag->hide();
      syncMatsUI.fresnel->hide();
      break;
    }
    case MatSkin: {
      syncMatsUI.topCoatFlag->hide();
      syncMatsUI.translucencyFlag->hide();
      syncMatsUI.sssFlag->show();
      syncMatsUI.fresnel->show();
      break;
    }
    default:
      break;
  }

  if ( dlg.exec() == QDialog::Rejected ) {
    return;
  }

  QString objectID = baseMat->getParent()->getInternalName();
  QVariant v;
  int numMats = toBeSynchronized.count();
  for (int i = 0; i < numMats; i++) {
    auto mat = toBeSynchronized[i];
    QString matName = mat->getName();
    // Textures to be synchronized
    QStringList channels;
    // Sync the Diffuse textures
    if (syncMatsUI.diffuseMap->isChecked()) {
      channels << "Kd";
    }
    // Sync the specular map
    if (syncMatsUI.specularTex->isChecked()) {
      channels << "Ks";
    }
    // Sync the glossiness map
    if (syncMatsUI.glossinessTex->isChecked()) {
      channels << "Kg";
    }
    // Sync the glossiness bump map
    if (syncMatsUI.bumpMapTex->isChecked()) {
      channels << "Bm";
    }

    // Do the texture syncing
    for (int i = 0; i < channels.count(); i++) {
      QString channelName = channels[i];
      // We preserve the name of the texture in the target material
      QString textureName;
      auto ch = mat->getChannel(channelName);
      if (!ch.isNull()) {
        textureName = ch->getName();
      }
      else {
        textureName = QString("%1_tex").arg(channelName);
      }
      ReTexturePtr t = baseMat->getNamedValue(channelName).value<ReTexturePtr>();
      if (!t.isNull()) {
        QVariant vt;
        vt.setValue<ReTexturePtr>(
          ReTexturePtr(
            ReTextureCreator::createTexture(textureName, t)
          )
        );
        auto t2 = vt.value<ReTexturePtr>();
        mat->addTextureToCatalog(t2);
        t2->reparent(mat);
        mat->setNamedValue(channelName, vt);
        mat->setEdited(true);
        QVariantMap args;
        args["objectID"] = objectID;
        args["sourceMaterialName"] = baseMat->getName();
        args["channelName"] = channelName;
        args["targetMaterialName"] = mat->getName();
        realityDataRelay->sendMessageToServer(
          SYNCHRONIZE_CHANNEL_TEXTURE, &args
        );
      }
      // If the source texture is blank then we need to erase the texture
      // in the target channel
      else {
        mat->setChannel(channelName,"");
        QVariantMap args;
        args["objectID"]     = objectID;
        args["materialName"] = baseMat->getName();
        args["channelName"]  = channelName;
        realityDataRelay->sendMessageToServer(UNLINK_TEXTURE, &args);
      }
    }

    // List of properties to be synchronized
    QStringList properties;
    // Sync the glossiness values
    if (syncMatsUI.glossiness->isChecked()) {
      properties <<  "uGlossiness";
      properties <<  "vGlossiness";
    }
    // Sync the bump map values
    if (syncMatsUI.bumpMap->isChecked()) {
      properties << "bmNegative";
      properties << "bmPositive";
      properties << "bmStrength";
    }
    // Sync the fresnel amount
    if (syncMatsUI.fresnel->isChecked()) {
      properties << "fresnelAmount";
    }
    // Sync the SSS flag
    if (syncMatsUI.sssFlag->isChecked()) {
      properties << "sssEnabled";
    }
    // Sync the top coat flag
    if (syncMatsUI.topCoatFlag->isChecked()) {
      properties << "topCoat";
    }
    // Sync the translucency coat flag
    if (syncMatsUI.topCoatFlag->isChecked()) {
      properties << "translucent";
    }
    // Sync the surface fuzz coat flag
    if (syncMatsUI.topCoatFlag->isChecked()) {
      properties << "surfaceFuzz";
    }

    for (int i = 0; i < properties.count(); i++) {
      QString propName = properties[i];
      v = baseMat->getNamedValue(propName);
      mat->setNamedValue(propName, v);
      mat->setEdited(true);
      updateHostMaterialProperty(objectID, matName, propName, v);      
    }
    // Finally sync the Diffuse map properties
    if (syncMatsUI.diffuseProps->isChecked()) {
      auto srcTex = findImageMapTexture(baseMat->getChannel("Kd"));
      if (!srcTex.isNull()) {
        auto targetTex = findImageMapTexture(mat->getChannel("Kd"));
        if (!targetTex.isNull()) {
          ReUpdateHostTextureAction act(mat->getParent()->getInternalName(),
                                        mat->getName(),
                                        "",
                                        targetTex);

          properties.clear();
          properties << "gain"  << "gamma"   << "uTile"
                     << "vTile" << "uOffset" << "vOffset"
                     << "mapping";
          for (int i = 0; i < properties.count(); i++) {
            QString prop = properties[i];
            auto val = srcTex->getNamedValue(prop);
            // Set the value on the UI side...
            targetTex->setNamedValue(prop, val);
            // Set the value on the host side
            act.setValue(prop,val);
            act.execute();
          }           
        }
      }
    }
  }
  container->showMessage(tr("Materials synchronized"));
  tvMaterials->update();
}

void RealityPanel::revertShaderToOriginal() {
  QVariantMap args;
  if (!currentObject) {
    return;
  }
  if (!currentMaterial) {
    return;
  }
  // Find what materials are selected for the conversion
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();
  int numRows = rows.count();

  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    if (mat) {
      args["objectID"] = mat->getParent()->getInternalName();
      args["materialName"] = mat->getName();
      realityDataRelay->sendMessageToServer(REVERT_SHADER_TO_ORIGINAL, &args);
    }
  }
}

int RealityPanel::savePresetVolume(const QString& objectID, 
                                   const QString& setID, 
                                   const QString& volumeName) {
  // Find if a copy of the volume is already stored in the ACSEL database
  int volumeID = findShaderVolumeInAcsel(objectID, setID, volumeName);
  // Convert the volume data to JSON
  ReQtVolumeExporter volExporter;
  QVariantMap volData = volExporter.exportVolume(
    RealitySceneData->getVolume(volumeName)
  );

  QJson::Serializer json;
  QString volumeCode = json.serialize(volData);
  volumeID = ReAcsel::getInstance()->saveVolume(volumeID, setID, volumeCode);
  return volumeID;
}

void RealityPanel::saveUniversalShader() {
  ReUniShaderSave uniShaderSaveDlg( currentMaterial, this );
  if ( !uniShaderSaveDlg.exec() ) {
    return;
  }

  auto acsel = ReAcsel::getInstance();
  QString shaderID        = uniShaderSaveDlg.getShaderID();
  QString shaderName      = uniShaderSaveDlg.getName();
  QString desc            = uniShaderSaveDlg.getDescription();
  QString author          = uniShaderSaveDlg.getAuthor();
  QString previewFileName = uniShaderSaveDlg.getPreviewFileName();
  QString category        = uniShaderSaveDlg.getCategory();

  auto matExporter = ReQtMaterialExporterFactory::getExporter(currentMaterial);
  boost::any result;
  matExporter->exportMaterial(currentMaterial, result);
  // Convert from Boost::any to QVariantMap
  QVariantMap qvmShaderData = boost::any_cast<QVariantMap>(result);
  // Serialize the data to store it in the database
  QJson::Serializer json;
  QString matDef = json.serialize(qvmShaderData);

  if (!acsel->saveUniversalShader(
         shaderID, shaderName, desc, matDef, category, author, previewFileName
       )) 
  {
    QMessageBox::information(
      this, 
      tr("Error saving Universal Shader"), 
      QString(
        tr("The following error occurred while saving the selected material"
           " as a universal shader: %1")
      )
      .arg(acsel->getErrorMessage())
    );
  }
  else {
    // RE_LOG_INFO() << "Universal shader ID: " << shaderID.toStdString();
    container->showMessage(tr("Universal shader saved"));
  }
}

void RealityPanel::applyUniversalShader() {
  // Find what materials are selected for the conversion
  QList<QModelIndex> rows = tvMaterials->selectionModel()->selectedRows();

  // Ask the user to select a shader set...
  ReUniversalShaderSelector shaderSelector(this);
  if ( shaderSelector.exec() == QDialog::Rejected ) {
    return;
  }

  auto shaderID = shaderSelector.getSelectedShaderID();
  container->showMessage(QString("Selected %1").arg(shaderID));
  int numRows = rows.count();
  for (int i = 0; i < numRows; i++) {
    ReMaterial* mat = sceneDataModel->getSelectedMaterial(
                         sortedSceneDataModel->mapToSource(rows[i])
                      );
    // It should never happen but you never know...
    if (!mat) {
      continue;
    }
    QString matName = mat->getName();
    QVariantMap args;
    
    args["objectID"] = mat->getParent()->getInternalName();
    args["matID"]    = matName;
    args["shaderID"] = shaderID;
    realityDataRelay->sendMessageToServer(APPLY_UNIVERSAL_SHADER, &args);
  }

}



