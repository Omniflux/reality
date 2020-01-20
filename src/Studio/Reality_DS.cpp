/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#include <QtCore>
#include <QtGlobal>
#include <QSettings>
#include <QMutableMapIterator>
#include <QSize>

#include "ReAppVersion.h"
#include "Reality_DS.h"
#include "ReAppVersion.h"
#include "RealityAction.h"
#include "RealityBase.h"
#include "ReSceneDataGlobal.h"
#include "ReIPC.h"
#include "ReDSMaterialConverter.h"
#include "ReDSStorage.h"
#include "ReGeometryExporter.h"
#include "ReDSTools.h"
#include "ReAcsel.h"
#include "ReRenderContext.h"
#include "ReDRM.h"
// #include "ReDSDebugTools.h"
#include "crc.h"

#ifdef __APPLE__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wall"
#endif

#include "dzplugin.h"
#include "dzversion.h"
#include "dzappsettings.h"
#include "dzscene.h"
#include "dznode.h"
#include "dzscene.h"
#include "dzspotlight.h"
#include "dzbricklight.h"
#include "dzfloatproperty.h"
#include "dzactionmgr.h"
#include "dzobject.h"
#include "dzfacetshape.h"
#include "dzmaterial.h"
#include "dzdefaultmaterial.h"
#include "dzfacetmesh.h"
#include "dzcustomdata.h"
#include "dzsettings.h"
#include "dzprogress.h"
#include "dzcontentmgr.h"
#include "dzimagemgr.h"
#include "dzrendermgr.h"
#include "dzrenderoptions.h"
#include "dzinstancenode.h"
#include "dzassetmgr.h"
#include "dzstringproperty.h"

#ifdef __APPLE__
  #pragma clang diagnostic pop
#endif

#include <iostream>

QString makeObjectUID( const DzNode* node );
void setGUIDForNode( DzNode* node, const QString& GUID );

bool isCamera( const DzNode* node ) {
  return node->inherits("DzCamera") && !node->inherits("DzLight");
}

bool isLight( const DzNode* node ) {
  return node->inherits("DzLight") 
         || node->inherits("DzShaderLight")
         || node->inherits("DzBrickLight");
}

// Static variable initialization
bool Reality_DS::nodeAdditionActive = true;

void Reality_DS::enableNodeAddition( const bool newState ) {
  nodeAdditionActive = newState;
}

#define RE_DEFAULT_WINDOWS_PATH "C:/Program Files/Reality_DS"
#define RE_DEFAULT_OSX_PATH     "/Applications/Reality_DS"

/*****************************
   Plugin Definition
*****************************/

static QString RealityDescription = QString(
 "This plugin is a component of Reality for DAZ Studio by Pret-A-3D.<br> "
 "Reality for DAZ Studio is copyright (c) 2010 by Pret-A-3D.<br>"
 "Reality plugin is a trademark by Pret-a-3D. All rights reserved.\n"
 "<p>Please visit <a href=\"http://preta3d.com\">preta3d.com</a> for the latest news about Reality</p>"
 "<p>Pret-A-3D would like to thank the developers of LuxRender, their vision, "
 "genius and generosity made having this kind of rendering for Studio "
 "possible.</p><p>Please visit "
 "<a href=\"http://www.luxrender.net\">luxrender.net</a> for all the "
 "latest news about Lux Render.</p>"
);

Reality_DS* Reality_DS::instance = NULL;

Reality_DS* Reality_DS::getInstance() {
  return instance;
}

Reality_DS::Reality_DS(): 
  DzPlugin(REALITY_PLUGIN_NAME, 
           "Pret-a-3D - Paolo Ciccone",
           RealityDescription,
           REALITY_MAIN_VERSION,
           REALITY_SUB_VERSION,
           REALITY_PATCH_VERSION, 
           REALITY_BUILD_NUMBER
           ) 
{ 
  dataSize = 0;
  sceneLevel = 0;
  isSceneLoading = false;
  sceneIsMerging = false;
  instance = this;
};

Reality_DS::~Reality_DS() {
}

/****************************************************
 * DAZ Studio entry point 
 ****************************************************/

DZ_CUSTOM_PLUGIN_DEFINITION( Reality_DS );   

DZ_PLUGIN_CLASS_GUID_OWNED( Reality_DS, 
                            EF4D89D5-2406-4C13-903D-20A6960E7C41,
                            QScriptEngine::ScriptOwnership );

// Action for Menu
DZ_PLUGIN_CLASS_GUID( Reality3Action, 717463F9-6616-4CE9-8370-A03F0AF757E2 );

//! Registering the class that implements storage of the scene data
DZ_PLUGIN_CLASS_GUID( ReSceneBlock, F5E4F9D0-98AA-4A54-9535-057086EC8619 );
DZ_PLUGIN_CLASS_GUID( ReStorage,  A780947C-3A11-4A24-B14D-F44709D2F22C );
DZ_PLUGIN_REGISTER_SCENEDATA_EXTRA_OBJECT_IO( "ReSceneDataBlock", 
                                              ReStorage, 
                                              ReSceneBlock );

/****************************************************/

void Reality_DS::checkForUpdates() {
};

void Reality_DS::installHooks() {
  connect(dzScene, SIGNAL(cameraRemoved(DzCamera*)), this, SLOT(cameraRemoved(DzCamera*)));
  connect(dzScene, SIGNAL(lightRemoved(DzLight*)), this, SLOT(lightRemoved(DzLight*)));
  connect(dzScene, SIGNAL(nodeAdded(DzNode*)), this, SLOT(nodeAdded(DzNode*)));
  connect(dzScene, SIGNAL(aboutToRemoveNode(DzNode*)), this, SLOT(nodeRemoved(DzNode*)));
  // Scene handling events
  connect(dzScene, SIGNAL(sceneLoadStarting()), this, SLOT(sceneLoadingStarted()));
  connect(dzScene, SIGNAL(sceneLoaded()), this, SLOT(sceneLoaded()));
  connect(dzScene, SIGNAL(sceneCleared()), this, SLOT(setNewScene()));

  // Update the library paths if the user installs new content
  auto contMgr = dzApp->getContentMgr();
  connect(contMgr, SIGNAL(contentDirectoryListChanged()), this, SLOT(updateLibraryPaths()));
  connect(contMgr, SIGNAL(poserDirectoryListChanged()), this, SLOT(updateLibraryPaths()));
}

void Reality_DS::removeHooks() {
  disconnect(dzScene, SIGNAL(cameraRemoved(DzCamera*)), this, SLOT(cameraRemoved(DzCamera*)));
  disconnect(dzScene, SIGNAL(lightRemoved(DzLight*)), this, SLOT(lightRemoved(DzLight*)));
  disconnect(dzScene, SIGNAL(nodeAdded(DzNode*)), this, SLOT(nodeAdded(DzNode*)));
  disconnect(dzScene, SIGNAL(aboutToRemoveNode(DzNode*)), this, SLOT(nodeRemoved(DzNode*)));
  disconnect(dzScene, SIGNAL(sceneLoadStarting()), this, SLOT(sceneLoadingStarted()));
  disconnect(dzScene, SIGNAL(sceneLoaded()), this, SLOT(sceneLoaded()));
  disconnect(dzScene, SIGNAL(sceneCleared()), this, SLOT(setNewScene()));
}

bool Reality_DS::verifyRealityInstallation( const QString& realityPath ) {
  QFileInfo realityInfo;
  if (osType == WINDOWS) {
    realityInfo.setFile(QString("%1/Reality.exe").arg(realityPath));
  }
  else if (osType == MAC_OS) {
    realityInfo.setFile(QString("%1/Reality.app/Contents/MacOS/Reality").arg(realityPath));
  }
  return realityInfo.exists();
}

void Reality_DS::updateLibraryPaths() {
  // Collect the paths of the content library of Studio and pass them to
  // Reality for path expansion of the textures
  auto contMgr = dzApp->getContentMgr();
  int numDirs = contMgr->getNumContentDirectories();
  QStringList dirs;
  for (int i = 0; i < numDirs; i++) {
    dirs << contMgr->getContentDirectoryPath(i);
  }
  numDirs = contMgr->getNumPoserDirectories();
  for (int i = 0; i < numDirs; i++) {
    dirs << contMgr->getPoserDirectoryPath(i);
  }
  RealityBase::getRealityBase()->setLibraryPaths(dirs);  
}

void Reality_DS::startup() { 
  // sceneIsMerging = false;

  RE_LOG_INFO() << "==================================================";
  RE_LOG_INFO() << " Reality plugin Studio Edition started";
  RE_LOG_INFO() << QString(" v. %1.%2.%3.%4")
                     .arg(REALITY_MAIN_VERSION) 
                     .arg(REALITY_SUB_VERSION)
                     .arg(REALITY_PATCH_VERSION)
                     .arg(REALITY_BUILD_NUMBER);
  RE_LOG_INFO() << "==================================================";

  RealityBase* rBase = RealityBase::getRealityBase();
  rBase->startHostSideServices(DAZStudio);
  rBase->setHostVersion(dzApp->getLongVersionStringProp());
  realityActive = false;
  shuttingDown = false;

  RE_LOG_INFO() << "Reality DS library v. " << REALITY_DS_LIBRARY_VERSION;

  osType = RealityBase::getRealityBase()->getOSType();
  QSettings configuration(RE_CFG_DOMAIN_NAME, RE_CFG_APPLICATION_NAME);
  realityPath = configuration.value(RE_CFG_REALITY_DS_LOCATION).toString();
  RE_LOG_INFO() << "Cfg file: " << configuration.fileName();
  RE_LOG_INFO() << "  realityPath: " << realityPath;
  bool realityFound = false;
  if (realityPath.isEmpty()) {
    RE_LOG_INFO() << "No location found for Reality. Trying to locate it automatically.";
    if (osType == WINDOWS) {
      realityPath = RE_DEFAULT_WINDOWS_PATH;
    }
    else if (osType == MAC_OS) {
      realityPath = RE_DEFAULT_OSX_PATH;
    }
  }
  realityFound = verifyRealityInstallation(realityPath);
  if (realityFound) {
    RE_LOG_INFO() << "Reality for DS found in " << QSS(realityPath);
    configuration.setValue(RE_CFG_REALITY_DS_LOCATION, realityPath);
    configuration.sync();
  }
  else {
    QMessageBox::information(
      0, 
      tr("Reality not found"),
      tr(
        "The Reality app could not be found in the standard location.\n"
        "You will be asked to locate the directory where the program is "
        "installed on your computer. "
      )
    );
    realityPath = QFileDialog::getExistingDirectory(
      0,
      "Chose the directory where Reality is installed",
      (osType == MAC_OS ? "/Applications" : "C:/Program Files")
    );
    realityFound = verifyRealityInstallation(realityPath);
    if (!realityFound) {
      QMessageBox::warning(
        0, 
        tr("Reality not found"),
        tr("I'm sorry but Reality could not be found in that location.")
      );
      return;
    }
    else {
      configuration.setValue(RE_CFG_REALITY_DS_LOCATION, realityPath);
      configuration.sync();      
    }
  }

  realityActive = realityFound;

  updateLibraryPaths();
  // Add the path to the Reality content, if needed
  installContentPath();

  installHooks();
  addSceneStorage();  
  scanScene( true );
  //********************************************************************
  // Find the 3D viewport and capture the signal to find when the user
  // switches active camera
  //********************************************************************
  Dz3DViewport* dsViewPort = dzApp->getInterface()->getViewportMgr()
                             ->getActiveViewport()->get3DViewport();
  connect(dsViewPort, SIGNAL(activeCameraChanged (DzCamera*)), 
          this, SLOT(selectActiveCamera(DzCamera*)));

  //********************************************************************
  // Start a timer to update camera data every second. There are no
  // reliable signals in Studio to do this on-demand.
  //********************************************************************
  cameraTimer = new QTimer(this);
  connect(cameraTimer, SIGNAL(timeout()), this, SLOT(processTimedEvents()));
  cameraTimer->start(RE_REALITY_TIMER_INTERVAL);
};

// We scan all the objects scheduled to be monitored, those are the
// objects loaded from the scene for whivh we have data saved. By scanning
// that list we focus the search on new objects and avoid reading data for
// objects that have been added to the scene in other ways. This is a possible
// scenerion because the same loading procedure is used whether we load a scene
// by itself or as a result of the user selecting File | Merge in DS.
//
// Once we found the object specified in objID, our target, we check if
// it has a GUID already. If it doesn't then we create a new GUID and return
// true.
bool Reality_DS::calcIDForObject( const QString& objID, QString& newID, QString& objLabel ) {
  int nodeCount = monitoringList.count();
  for (int i = 0; i < nodeCount; i++) {
    auto node = dzScene->getNode(monitoringList[i]);
    if (!node) {
      return false;
    }
    QString oid;
    // Tests need to be done in this sequence because DzLight inherits from DzCamera
    if (node->inherits("DzLight")) {
      oid = ReGUID::getGUID(node);
      if (oid == ReGUID::InvalidGUID) {
        oid = node->getName();
      }
    }
    else if (node->inherits("DzCamera")) {
      oid = ReGUID::getGUID(node);
      if (oid == ReGUID::InvalidGUID) {
        oid = node->getAssetId();
      }
    }
    else {
      // This is an object
      oid = ReGUID::getGUID(node);
    }
    if (oid == objID) {
      newID = makeObjectUID(node);
      objLabel = node->getLabel();
      setGUIDForNode(node, newID);
      return true;
    }
  }
  return false;
};

void Reality_DS::installContentPath() {
  auto config = RealityBase::getConfiguration();
  QString cfgUpdateKey = "configurationUpdated";
  if (!config->value(cfgUpdateKey,false).toBool()) {
    QString configKey = "DS_ContentDir";
    auto contentPath = config->value(configKey,"").toString();
    if (!contentPath.isEmpty()) {
      auto contentMrg = dzApp->getContentMgr();
      if (!contentMrg) {
        return;
      }

      contentMrg->addContentDirectory(contentPath);
      config->setValue(cfgUpdateKey, true);
    }
  }
}


void Reality_DS::shutdown() {
  shuttingDown = true;
  if (getStatus() != Loaded) {
    return;
  }

  removeHooks();
  realityIPC->closeGUI();
  RealityBase::getRealityBase()->stopHostSideServices();  
};

//! Add one instance of our scene data object that is used by the Studio
//! API to trigger the saving and loading of customer's data. See the
//! \ref ReDS_Storage class for the actual implementation of the input/output
//! interface
void Reality_DS::addSceneStorage() {
  dzScene->addDataItem(new ReSceneBlock());
};

void Reality_DS::sceneLoadingStarted() {
  RE_LOG_INFO() << "Loading a new scene...";
  if (sceneLevel == 0) {
    ReAcsel::getInstance()->startCaching();
  }
  sceneLevel++;
  isSceneLoading = true;
  // We assume that we merge the scenes. If we are not then the setNewScene()
  // method will be called before any processing starts and that method
  // will set the sceneIsMerging flag to false
  sceneIsMerging = true;
}

void Reality_DS::sceneLoaded() {
  // if (isSceneLoading) {
  //   RE_LOG_INFO() << "  Merging Studio scenes";    
  // }
  // else {
  //   RE_LOG_INFO() << "  No scene merging needed";
  // }
  sceneLevel--;
  if (sceneLevel == 0) {
    dzApp->postEvent(this, new ReStudioEvent("addNode") );    
    isSceneLoading = false;
    auto camID = getCurrentStudioCamera();

    RealitySceneData->selectCamera(camID);  
    realityIPC->hostCameraSelected(camID);

    // connect with the Reality IBL sphere, if present, to track changes
    auto iblSphere = dzScene->findNodeByLabel(REALITY_IBL_SPHERE);
    if (iblSphere) {
      connect(iblSphere, SIGNAL(transformChanged()), 
        this, SLOT(changeIBLRotation()));
    }
  }
}

void Reality_DS::addNode( ReStudioEvent* reEvent ) {
  RE_LOG_INFO() << "File or group finished loading";
  int numNodes = nodeList.count();
  for (int i = 0; i < numNodes; i++) {
    addNode(nodeList[i]);
  }
  nodeList.clear();
  // Add the monitors for objects loaded from the Studio scene for 
  // which Reality data was loaded.
  numNodes = monitoringList.count();
  for (int i = 0; i < numNodes; i++) {
    monitorNode(dzScene->getNode(monitoringList[i]));
  }
  monitoringList.clear();

  if ( reEvent->shouldCloseEventQueue() && sceneLevel == 0 ) {
    isSceneLoading = false;
  }
  ReAcsel::getInstance()->stopCaching();      
}

void Reality_DS::addNode( DzNode* node ) {
  QString objID = makeObjectUID(node);
  if (objID == ReGUID::InvalidGUID) {
    return;
  }
  bool isLightNode = isLight(node);
  bool isCameraNode = isCamera(node);

  // Skip nodes that :
  //   - are parented and hidden, those are generally controllers
  //   - don't have geometry, unless they are instances
  bool skip = false;
  
  if (!isCameraNode && !isLightNode) {
    skip = (node->parent() && node->isHidden()) ||
           node->inherits("DzGroupNode");

    if (!node->getObject()) {
      if (!node->inherits("DzInstanceNode")) {
        skip = true;
      }
    }
  }
  if (!skip) {
    setGUIDForNode(node, objID);
    if (isLightNode) {
      lightAdded( static_cast<DzLight*>(node) );    
    }
    else if ( isCameraNode ) {
      cameraAdded(qobject_cast<DzCamera*>(node));
    }
    else {
      addRealityObject(objID, node);
    }
    monitorNode(node);
  }
  else {
    RE_LOG_INFO() << "Object " << node->getName()
    << " skipped."
    << QString(" - parented&hidden:%1, group: %2, no geom: %3, inst.: %4")
    .arg((node->parent() && node->isHidden()))
    .arg(node->inherits("DzGroupNode"))
    .arg(node->getObject() == NULL)
    .arg(node->inherits("DzInstanceNode"))
    ;
  }
}


void Reality_DS::updateMaterial() {
  ReAcsel::getInstance()->startCaching();

  auto matConverter = ReDSMaterialConverter::getInstance();
  QMutableHashIterator<QString, int> i(materialRefreshTable);
  realityIPC->GUIPauseMatPreview();
  while( i.hasNext() ) {
    i.next();
    QStringList key = i.key().split("::");
    DzMaterial* mat= DzMaterial::getMaterial(i.value());
    if (!mat) {
      continue;
    }
    auto matInfo = matConverter->convertMaterial(mat);
    RealitySceneData->updateMaterial(key[0], key[1], *matInfo);
    auto reMat = RealitySceneData->getMaterial(key[0],key[1]);
    flatMatList.storeMaterial(mat->getIndex(), reMat);
    i.remove();
  }
  ReAcsel::getInstance()->stopCaching();

  realityIPC->GUIResumeMatPreview();
}

void Reality_DS::updateMaterialList() {
  int numMaterials = DzMaterial::getNumMaterials();
  for (int i=0; i < numMaterials; i++) {
    auto dsMat = DzMaterial::getMaterial(i);    
    if (!dsMat) {
      continue;
    }
    auto shapes = dsMat->shapeListIterator();
    while( shapes.hasNext() ) {
      auto shape = shapes.next();
      if (!shape) {
        continue;
      }
      auto node = shape->getNode();
      QString objID = ReGUID::getGUID(node);
      auto reMat = RealitySceneData->getMaterial(objID, dsMat->getName());
      if (!reMat.isNull()) {
        flatMatList.storeMaterial(i, reMat);
      }
    }
  }
}

bool Reality_DS::event( QEvent* e ) {

  if (e->type() != ReStudioEvent::reEventType) {
    return false;
  }

  auto reEvent = static_cast<ReStudioEvent*>(e);
  QString eventDesc = reEvent->getDescription();
  bool result = false;
  // Adding a node
  if ( eventDesc == "addNode" ) {
    addNode(reEvent);
    result = true;
  }
  // Material has changed
  else if ( eventDesc == "materialChanged" ) {
    if (materialRefreshTable.count()) {
      updateMaterial();
    }
    result = true;
  }
  else if (eventDesc == "lightPos") {
    auto light = boost::any_cast<DzLight*>(reEvent->getProperty("light"));
    if (light) {    
      DzMatrix3 m = light->getWSTransform();
      ReMatrix lightMatrix;
      convertDzMatrix(m, lightMatrix);
      RealitySceneData->setLightMatrix(ReGUID::getGUID(light), lightMatrix);
    }
  }

  return result;
}

void Reality_DS::setNewScene() {
  addSceneStorage();
  flatMatList.clear();
  sceneIsMerging = false;
  // RE_LOG_INFO() << "New Studio scene";
  if (isSceneLoading || dzApp->isClosing()) {
    return;
  }
  isSceneLoading = false;
  RealitySceneData->newScene();
  realityIPC->setNewScene();
  // RE_LOG_INFO() << "  Reality scene cleared";
  scanScene();
}

void Reality_DS::setFrameSize( const QSize& newSize ) {
  RealitySceneData->setFrameSize(newSize.width(), newSize.height());  
}

void Reality_DS::scanScene( const bool installMonitor ) {
  QListIterator<DzCamera*> cameraIter = dzScene->cameraListIterator();
  while( cameraIter.hasNext() ) {
    DzCamera* dzCam = cameraIter.next();
    cameraAdded(dzCam);
  }
  // Get the render options and install a monitor to track 
  // resolution changes
  auto renderMgr = dzApp->getRenderMgr();
  if (!renderMgr) {
    return;
  }
  auto renderOpts = renderMgr->getRenderOptions();
  setFrameSize(renderOpts->getImageSize());
  if ( installMonitor ) {
    connect( renderOpts, SIGNAL(imageSizeChanged(const QSize&)), 
             this, SLOT(setFrameSize(const QSize&)) );
  }
}

void setCameraData( const DzCamera* dzCam, ReCameraPtr reCam ) {
  reCam->setFocalLength(dzCam->getFocalLength());
  reCam->setFocalDistance(dzCam->getFocalDistance()/100.0);
  reCam->setName(dzCam->getLabel());
  // Studio returns the FOV in radians, we use degrees
  reCam->setFOV( dzCam->getFieldOfView()*180/PI );
  DzMatrix3 m = dzCam->getWSTransform();
  ReMatrix reMatrix;
  convertDzMatrix(m, reMatrix);
  reCam->setMatrix(reMatrix);
}

void Reality_DS::cameraAdded( DzCamera* dzCam ) {
  // QString camID = dzCam->getAssetId();
  QString camID = ReGUID::getGUID(dzCam);
  if (camID == ReGUID::InvalidGUID) {
    camID = makeObjectUID(dzCam);
    setGUIDForNode(dzCam, camID);
  }

  ReCameraPtr cam = ReCameraPtr( 
    new ReCamera(dzCam->getLabel(), camID)
  );
  // set the parameters
  setCameraData(dzCam, cam);
  RealitySceneData->addCamera(cam);
  // Store the camera data for caching
  cameraData[camID] = cam->toString();
  monitorCamera(dzCam);
};

void Reality_DS::cameraRemoved( DzCamera* camera ) {
  if (shuttingDown) {
    return;
  }
  auto camID = ReGUID::getGUID(camera);
  RealitySceneData->removeCamera(camID);
  // RE_LOG_INFO() << "Camera removed: " << camID;
  cameraData.remove(camID);
  camID = getCurrentStudioCamera();
  if (camID != "") {
    // RE_LOG_INFO() << "Studio current camera: " << camID;
    RealitySceneData->selectCamera(camID);
    realityIPC->hostCameraSelected(camID);
  }
};

void Reality_DS::cameraRenamed( const QString& newLabel ) {
  if (shuttingDown) {
    return;
  }  
  DzCamera* dsCam = static_cast<DzCamera*>(QObject::sender());
  RealitySceneData->renameCamera(ReGUID::getGUID(dsCam), newLabel);
};

void Reality_DS::renderFrame( const QString& sceneFileName, 
                              const unsigned int frameNo,
                              const bool runRenderer ) 
{
  QStringList instances;

  RealitySceneData->renderSceneStart(sceneFileName, frameNo);

  auto exporter = new DS::ReGeometryExporter(&flatMatList);
  auto objs = exporter->getNodes();
  DS::ReGeometryExporter::NodeDictIterator nodeIter(objs);

  DzProgress progressInd("Rendering", objs.count(), false, true);
  realityIPC->exportStarted(objs.count());

  while( nodeIter.hasNext() ) {
    nodeIter.next();
    QString objName = nodeIter.key();
    RE_LOG_INFO() << "Exporting object " << objName;
    auto obj = RealitySceneData->getObject(objName);
    if (obj.isNull()) {
      continue;
    }
    QString msg = QString("Exporting to LuxRender %1").arg(objName);
    if (obj->isInstance()) {
      instances.append(objName);
      continue;
    }
    realityIPC->exportingObject(msg);
    progressInd.setInfo(msg);
    progressInd.step();

    exporter->exportObject( objName, obj );
  }
  // Export the instances
  int numInstances = instances.count();
  for (int l = 0; l < numInstances; l++) {
    QString objName = instances[l];
    // We need to use the label to locate the object in the scene
    auto node = dzScene->findNodeByLabel(
      RealitySceneData->getObject(objName)->getName()
    );
    if (!node) {
      continue;
    }
    auto target = static_cast<DzInstanceNode*>(node)->getTarget();
    if (!target) {
      continue;
    }
    auto tm = target->getWSTransform();
    auto im = node->getWSTransform();

    DzMatrix3 relMatrix = tm.inverse() * im;

    ReMatrix reMatrix;
    convertDzMatrix(relMatrix, reMatrix);
    RealitySceneData->renderSceneExportInstance(objName, reMatrix, DAZStudio);    

  }

  RealitySceneData->renderSceneFinish(runRenderer);

  realityIPC->exportFinished();  
  delete exporter;
};

void Reality_DS::renderCurrentFrame( const QString& sceneFileName, 
                                     const unsigned int frameNo,
                                     const bool runRenderer ) 
{
  QTime t;
  t.start();
  ReRenderContext::getInstance()->init();
  renderFrame(sceneFileName, frameNo, runRenderer);
  RE_LOG_INFO() << "Elapsed Time: " << t.elapsed()/1000.0  << " sec.";  
}

void Reality_DS::getAnimationRange( int& startFrame, int& endFrame, int& fps ) {
  // Amount of clock "ticks" in a second, in Studio
  unsigned int animDivisor = 4800.f;
  auto animRange           = dzScene->getAnimRange();
  float frameDuration      = dzScene->getTimeStep();
  fps                      = animDivisor/frameDuration;
  startFrame               = animRange.getStart();
  endFrame                 = animRange.getEnd()/frameDuration;  
}

void Reality_DS::renderAnimation( const bool runRenderer,
                                  int selectedStartFrame, 
                                  int selectedEndFrame ) 
{
  int startFrame,
      endFrame,
      fps;

  getAnimationRange(startFrame, endFrame, fps);

  if (selectedStartFrame > 0) {
    startFrame = selectedStartFrame;
  }
  if (selectedEndFrame != 0) {
    endFrame = selectedEndFrame;
  }

  // Save the current frame for restoring it later
  int currentFrame = dzScene->getFrame();

  RE_LOG_INFO() << QString("Animation: start: %1, stop: %2, fps: %3")
                     .arg(startFrame).arg(endFrame).arg(fps);

  QString sceneName = RealitySceneData->getSceneFileName();

  QStringList renderQueue;
  // Render the animation
  QProgressDialog progress(dzApp->getInterface());
  progress.setLabelText(tr("Reality is exporting your animation..."));
  progress.setRange(startFrame, endFrame);
  progress.setModal(true);
  bool renderWasInterrupted = false;
  for (int i = startFrame; i <= endFrame; i++) {
    progress.setValue(i);
    dzScene->setFrame(i);
    dzApp->processEvents();
    if (progress.wasCanceled()) {
      renderWasInterrupted = true;
      break;
    }
    updateCameraData();
    QString fileName = expandFrameNumber(sceneName, i);
    renderFrame(fileName, i, false);
    renderQueue << fileName;
  }
  dzScene->setFrame(currentFrame);
  updateCameraData();

  if (renderWasInterrupted) {
    return;
  }

  if (runRenderer) {
    ReLuxRunner rr;
    rr.runGuiLux( renderQueue, 
                  RealitySceneData->getNumThreads(), 
                  RealitySceneData->getLuxLogLevel() );
  }
};

DzMaterial* findNodeMaterial( const DzNode* node, const QString& matName ) {
  DzObject* obj = node->getObject();

  if (!obj) {
    RE_LOG_INFO() << "No object for node " << node->getName();
    return NULL;
  }

  DzShape* shape = obj->getCurrentShape();
  if (!shape) {
    RE_LOG_INFO() << "No shapes available for node " << node->getName(); 
    return NULL;
  }

  int numMats = shape->getNumAssemblyMaterials();

  for ( int l = 0; l < numMats; l++) {
    DzMaterial* theMat = shape->getAssemblyMaterial(l);
    if (theMat->getName() == matName) {
      return theMat;
    }
  }
  return NULL;
}

void Reality_DS::processTimedEvents() {
  //! Update the camera information
  updateCameraData();
  updateAnimationLimits();

  // Check if there are commands pending from the GUI to us
  RealityBase* rBase = RealityBase::getRealityBase();
  while( rBase->getNumCommands() ) {
    QString cmd = rBase->commandStackPop();
    if (cmd == "changed") {
      dzScene->markChanged();
    }
    else if (cmd == "render") { 
      QString runRender = rBase->commandStackPop();
      renderCurrentFrame( "", 0, runRender == "1" );
    }
    else if (cmd == "renderAnim") { 
      QString runRender = rBase->commandStackPop();
      int startFrame = rBase->commandStackPop().toInt();
      int endFrame = rBase->commandStackPop().toInt();
      renderAnimation( runRender == "1", startFrame, endFrame );
    }
    else if (cmd == "sip") { 
      // Set IBL Preview
      QString mapName = rBase->commandStackPop();
      setIBLPreviewMap(mapName);
    }
    else if (cmd == "save") {
      auto sceneName = dzScene->getFilename();
      if (sceneName == "") {
        sceneName = dzScene->getAssetLoadPath();
      };
      auto mainWindow = dzApp->getInterface();
      if (sceneName == "") {
        mainWindow->doFileSaveAs();
      }
      else {
        mainWindow->doFileSave();            
      }
    }    
    else if (cmd == "smha") {
      // Select material in host app
      QString objectID = rBase->commandStackPop();
      QString materialName = rBase->commandStackPop();
      selectStudioMaterial(objectID, materialName);
    }
  }

}

void Reality_DS::updateCameraData() {
  QListIterator<DzCamera*> cameraIter = dzScene->cameraListIterator();
  while( cameraIter.hasNext() ) {
    DzBasicCamera* dzCam = static_cast<DzBasicCamera*>(cameraIter.next());
    // QString camID = dzCam->getAssetId();
    QString camID = ReGUID::getGUID(dzCam);
    ReCameraPtr cam = RealitySceneData->getCamera(camID);
    if (cam) {
      setCameraData(dzCam, cam);
      QString camInfo = cam->toString();
      if (camInfo != cameraData[camID]) {
        cameraData[camID] = camInfo;
        realityIPC->cameraDataChanged(camID);
      }
    }
  }
}

void Reality_DS::updateAnimationLimits() {
  int startFrame, endFrame, fps;
  getAnimationRange(startFrame, endFrame, fps);

  realityIPC->updateAnimationLimits(startFrame, endFrame, fps);
}

void Reality_DS::selectActiveCamera(DzCamera* dsCam) {
  if (dzApp->isClosing()) {
    return;
  }
  // auto camID = dsCam->getAssetId();  
  auto camID = ReGUID::getGUID(dsCam);  
  // RE_LOG_INFO() << "DS calls for camera change: " << camID;
  RealitySceneData->selectCamera(camID);
  realityIPC->hostCameraSelected(camID);
}

void getLightInfo( DzLight* light, QVariantMap& data ) {
  QString lightID = ReGUID::getGUID(light);
  if (lightID == ReGUID::InvalidGUID) {
    lightID = makeObjectUID(light);
    setGUIDForNode(light, lightID);
  }
  data["name"]       = light->getLabel();
  data["lightID"]    = lightID;
  data["on"]         = light->isOn();
  data["colorModel"] = ReLight::Temperature;
  // Set the color data
  QVariantMap color;
  QVariantList theColor;

  auto colorProp = qobject_cast<DzColorProperty*>(
                     light->findProperty(RE_DS_LIGHT_COLOR)
                   );
  QColor lightColor;
  if (colorProp) {
    lightColor = colorProp->getColorValue();
    if ( !isPureWhite(lightColor) ) {
      data["colorModel"] = ReLight::RGB;
    }
  }
  else {
    lightColor = QColor(255,255,255);
  }
  theColor << QVariant(lightColor.redF()) 
           << QVariant(lightColor.greenF()) 
           << QVariant(lightColor.blueF());
  color["color"] = theColor;
  color["map"]   = QVariant("");
  data["color"]  = color;

  if (light->inherits("DzSpotLight")) {
    data["type"] = SpotLight;
    data["angle"] = (static_cast<const DzSpotLight*>(
      light
    ))->getSpreadAngleControl()->getValue();
  }
  else if (light->inherits("DzPointLight")) {
    data["type"] = PointLight;
  }
  else if (light->inherits("DzDistantLight")) {
    data["type"] = InfiniteLight;
  }
  else if (light->inherits("DzShaderLight")) {
    // Is this a fake spot
    auto coneProp = qobject_cast<DzFloatProperty*>(
                      light->findProperty("Cone Inside")
                    );
    if (coneProp) {
      data["type"] = SpotLight;
    }
    else {
      // Is this a fake pointlight?
      auto pointProp = qobject_cast<DzFloatProperty*>(
                        light->findProperty("Decay")
                      );
      if (pointProp) {
        data["type"] = PointLight;
      }
      else {
        data["type"] = InfiniteLight;
      }
    }
  }
  else if (light->isAreaLight()) {
    data["type"] = MeshLight;
  }
  data["intensity"] = 1.0;
  // Getting the intensity of the light
  // All lights, except for "Bricklight", subclass DzDistantLight
  if (light->inherits("DzDistantLight")) {
    data["intensity"] = (static_cast<const DzDistantLight*>(light))->getIntensity();
  }
  else if (light->inherits("DzBrickLight")) {
    data["intensity"] = (static_cast<const DzBrickLight*>(light))->getIntensity();
  }
}

void Reality_DS::lightAdded( DzLight* light ) {
  if (light->getName().startsWith(RE_DS_HEADLAMP_BLOCKER)) {
    return;
  }
  QVariantMap data;  
  getLightInfo(light, data);
  // Add the light to the scene and notify the UI
  RE_LOG_INFO() << "Adding light with ID " << data.value("lightID").toString();
  RealitySceneData->addLight(data.value("lightID").toString(), data);

  // Set the initial matrix
  DzMatrix3 m = light->getWSTransform();
  ReMatrix lightMatrix;
  convertDzMatrix(m, lightMatrix);
  RealitySceneData->setLightMatrix(data.value("lightID").toString(), 
                                   lightMatrix);
  monitorLight(light);
};

void Reality_DS::lightRemoved( DzLight* light ) {
  RealitySceneData->deleteLight(ReGUID::getGUID(light));
};

void Reality_DS::lightRenamed( const QString& newLabel ) {
  DzLight* dsLight = static_cast<DzLight*>(QObject::sender());
  RealitySceneData->renameLight(ReGUID::getGUID(dsLight), newLabel);
};

void Reality_DS::updateLightPos() {
  // Because Studio dispatches the events to plugins in the middle of their
  // execution, there is at least one situation where asking for data about
  // a light causes a crash in Studio. That happens when a light is set to 
  // point to an object, like a null, and then the object that it points to
  // is deleted. If the DzLight::getWSTransform() method is called in the middle
  // of processing the signal then Studio will crash. It seems that Studio
  // fires the signal and then completes the update of the light after the
  // signal is processed. Unfortunatelu the state of the light is such that
  // retrieving its coordinates causes the crash. 
  //
  // To avoid that issue we simply append an event to the app queue so that
  // we return to this operation once Studio is done. See the event() method.
  //
  auto light = dynamic_cast<DzLight*>(QObject::sender());
  auto reEvent = new ReStudioEvent("lightPos");
  boost::any lightPtr = light;
  reEvent->addProperty("light", lightPtr);
  dzApp->postEvent(this, reEvent);
}

void Reality_DS::updateLightProperty() {  
  auto prop = qobject_cast<DzProperty*>(QObject::sender());
  auto light = qobject_cast<DzLight*>(prop->getOwner());
  QString propName = prop->getName();
  QVariantMap data;
  getLightInfo(light, data);
  RealitySceneData->updateLight(ReGUID::getGUID(light), data);
}

void Reality_DS::selectStudioMaterial( const QString& objID, 
                                       const QString& materialName )
{
  auto obj = RealitySceneData->getObject(objID);

  if (obj.isNull()) {    
    return;
  }
  auto dsNode = dzScene->findNodeByLabel(obj->getName());
  if (!dsNode) {
    RE_LOG_WARN() << "Could nod find node " << dsNode->getName();
    return;
  }

  DzObject* dsObj = dsNode->getObject();

  if (!obj) {
    RE_LOG_INFO() << "No object found!";
    return;
  }

  DzShape* shape = dsObj->getCurrentShape();
  if (!shape) {
    RE_LOG_INFO() << "Object has no shape!";
    return;
  }

  // The surface selection tool must be on or this will not work
  auto vpMgr = dzApp->getInterface()->getViewportMgr();
  auto sst = vpMgr->findTool("DzMaterialPickTool");
  if (sst) {
    vpMgr->setActiveTool(sst);
  }

  // Deselect any previously selected material
  int numMats = DzMaterial::getNumMaterials();
  for( int i = 0; i < numMats; i++ ) {
   auto curMat = DzMaterial::getMaterial( i );
   if( curMat && curMat->isSelected() )
    curMat->select( false );
  }

  numMats = shape->getNumAssemblyMaterials();

  for ( int l = 0; l < numMats; l++) {
    DzMaterial* dsMat = shape->getAssemblyMaterial(l);
    if (dsMat->getName() == materialName) {
      dsMat->select(true);
    }
  }

};


void Reality_DS::nodeAdded( DzNode* node ) {
  // RE_LOG_INFO() << "Studio added " << node->getLabel()
  //               << "/" << node->getName()
  //               << " of class: " << node->metaObject()->className()
  //               << (node->metaObject()->superClass() ? 
  //                     QString("  superclass: %1")
  //                       .arg(node->metaObject()->superClass()->className())
  //                        :
  //                     "" )
  //               << ". Parented? " << (node->getNodeParent() != NULL)
  //               << " " 
  //               << (node->getNodeParent() != NULL ? node->getNodeParent()->getName() : ""); 

  // The following flag is used when we restore Reality data from the
  // saved scene. In that case all the data is already there and
  // we don't need to go through the process of adding nodes. We do
  // need to set the monitoring for all objects involved, though, and that has
  // to be delayed until Studio has completely created the node, which is not
  // done at this stage.
  if (!nodeAdditionActive) {
    monitoringList << dzScene->findNodeIndex(node);
    return;
  }
  bool shouldAddNode = true;
  QString nodeClass = node->metaObject()->className();
  // Lights are added by separate methods. 
  // Bones are not used anymore, they were used by DS3.
  if ( 
       node->inherits("DzMeasureMetricsNode") ||
       node->inherits("DzRigidFollowNode")    ||
       nodeClass == "DzBone"                  ||
       nodeClass == "DzIKNode"                ||
       // This includes DzDFormZone and 
       // DzDFormBase
       nodeClass.startsWith("DzDForm")        ||
       !node->isVisible()                     ||
       node->getName() == RE_DS_DATANODE_NAME ||
       node->getName() == RE_DS_METADATA 
     ) 
  {
    shouldAddNode = false;
  }

  // // Add lights
  // if ( shouldAddNode && isLight(node) ) {
  //   lightAdded( static_cast<DzLight*>(node) );
  //   return;
  // }
  // // Add cameras
  // if ( shouldAddNode && isCamera(node) ) {
  //   cameraAdded(qobject_cast<DzCamera*>(node));
  //   return;
  // }

  // Check if the node is one of the primitives. If
  // so we add the node
  QString nodeName = node->getName();
  if (  
       ( nodeName.startsWith("plane")    ||
         nodeName.startsWith("torus")    ||
         nodeName.startsWith("sphere")   ||
         nodeName.startsWith("cone")     ||
         nodeName.startsWith("cylinder") ||
         nodeName.startsWith("cube") ) && !isSceneLoading) 
  {
    // Direct addition because primitives are not loaded from a scene
    addNode(node);
    return;
  }

  // If a group is added while the scene is not loading this means
  // that any instance group has been created by the user.
  // This kind of group behaves in a slightly different way than the
  // a group coming from a loaded scene. So in this case, we fake
  // the condition of the scene loading, and we push an event in the
  // queue just to close the operation and reset the flag so that
  // we return to the standard condition.
  if (node->inherits("DzGroupNode") && !isSceneLoading) {
    RE_LOG_INFO() << "Instance group";
    isSceneLoading = true;
    dzApp->postEvent( this, new ReStudioEvent("addNode",true) );
    return;
  }

  if (shouldAddNode) {
    if (isSceneLoading) {
      nodeList.append(node);
    }
    else {
      // This happens usually when the user undoes a deletion, with the
      // result that the object is added back to the scene but there is 
      // no loading of a file, and so the isSceneLoading flag is not set

      // if ( isCamera(node) ) {
      //   cameraAdded(qobject_cast<DzCamera*>(node));
      // }
      // else if ( isLight(node) ) {
      //   lightAdded( static_cast<DzLight*>(node) );
      // }
      // else {
        addNode(node);
      // }
    }
  }
};

void Reality_DS::nodeRemoved( DzNode* node )  {
  if (shuttingDown) {
    return;
  }
  // A lot of calls are for nodes of thime DzBone, which we don't add to the
  // Reality database, so we can skip those right away.
  QString className = node->metaObject()->className();
  if ( className == "DzBone" || className.startsWith("DzDForm") ) {
    return;
  }
  // RE_LOG_INFO() << "Asked to remove node: " << node->getName()
  //               << " class: " << node->metaObject()->className()
  //               << " with UID: " << ReGUID::getGUID(node);
  int nodePos = nodeList.indexOf(node);
  if (nodePos != -1) {
    nodeList.removeAt(nodePos);
  }
  auto id = ReGUID::getGUID(node);
  if (id != ReGUID::InvalidGUID) {
    RealitySceneData->deleteObject(id);
  }
};


void Reality_DS::nodeRenamed( const QString& newLabel ) {
  DzNode* node = qobject_cast<DzNode*>(QObject::sender());
  QString objID = ReGUID::getGUID(node);
  if (RealitySceneData->hasObject(objID)) {
    RealitySceneData->renameObject(objID, newLabel);
  }
}

/*************************************************
 * Material monitoring
 *************************************************/

// Material definition changed
void Reality_DS::watchMaterial() {
  if (dzApp->isClosing() || dzScene->isClearing()) {
    return;
  }
  auto prop = qobject_cast<DzProperty*>(QObject::sender());
  if (!prop) {
    return;
  }
  auto propOwner = prop->getOwner();
  if (propOwner->inherits("DzMaterial")) {      
    auto mat = qobject_cast<DzMaterial*>(propOwner);
    auto i = mat->shapeListIterator();
    while( i.hasNext() ) {
      auto node = i.next()->getNode();
      if (node) {
        QString objID = ReGUID::getGUID(node);
        QString matName = mat->getName();
        // Get the Reality material to check of what type it is
        auto reMat = RealitySceneData->getMaterial(objID, matName);
        if (reMat && reMat->getType() == MatLight) {
          // If the material has been previously changed to be a light
          // then ignore the change int he host, it does not apply to our
          // material.
          return;
        }
        QString key = QString("%1::%2").arg(objID).arg(matName);
        // See event()
        materialRefreshTable[key] = mat->getIndex();
      }
    }
    dzApp->postEvent( this, new ReStudioEvent("materialChanged") );
  }
  else {
    RE_LOG_INFO() << QString(">> Property is of type: %1")
                     .arg(prop->metaObject()->className());
  }
}

// Material is selected in Studio, we select in the Reality UI as well
void Reality_DS::materialSelected(DzMaterial *mat, bool onOff) {
  if (onOff) {
    DzShapeListIterator sli = mat->shapeListIterator();
    DzNode* node = NULL;
    while( sli.hasNext() ) {
      auto shape = sli.next();
      if (node = shape->getNode()) {
        break;
      }
    }
    if (node) {
      realityIPC->materialSelected(ReGUID::getGUID(node), mat->getName());
    }
  }
}

void Reality_DS::visibilityChanged() {
  auto node = qobject_cast<DzNode*>(QObject::sender());
  auto objID = ReGUID::getGUID(node);
  if (objID != ReGUID::InvalidGUID) {
    RealitySceneData->setObjectVisible(objID, node->isVisible());
  }
}

void Reality_DS::setIBLPreviewMap( const QString& mapFilename ) {
  // Is the preview sphere already in the scene?
  auto previewSphere = dzScene->findNodeByLabel(REALITY_IBL_SPHERE);
  auto contentMgr = dzApp->getContentMgr();
  if (!previewSphere) {
    QString filePath = contentMgr->getAbsolutePath(
      "/Props/Reality/Reality IBL Sphere.duf",
      true
    );
    if (!filePath.isEmpty()) {
      if (!contentMgr->openFile(filePath)) {
        return;
      }
      // We need to be notified if the rotation of the sphere changes
      // so that we can reflect that in the IBL settings
      previewSphere = dzScene->findNodeByLabel(REALITY_IBL_SPHERE);
      if (!previewSphere) {
        return;
      }
      connect(previewSphere, SIGNAL(transformChanged()), 
              this, SLOT(changeIBLRotation()));
    }
  }
  // If the sphere has been added successfully to the scene then we
  // set the map for it and start tracking the changes to its rotation
  if (!previewSphere) {
    return;
  }
  auto obj = previewSphere->getObject();
  if (!obj) {
    return;
  }
  auto shape = obj->getCurrentShape();
  if (!shape) {
    return;
  }
  auto mat = qobject_cast<DzDefaultMaterial*>(shape->getMaterial(0));
  if (mat) {
    DzTexture* tex = dzApp->getImageMgr()->getImage(mapFilename);
    mat->setColorMap(tex);
  }
}

void Reality_DS::changeIBLRotation() {
  DzNode* source = qobject_cast<DzNode*>(QObject::sender());
  RealitySceneData->setIBLRotation(
    source->getYRotControl()->getValue()
  );
}

void Reality_DS::setIBLImage( const QString& mapFilename ) {
  RealitySceneData->setIBLImage(mapFilename);
}

//! Proxy method to forward it to the one in ReSceneData  
void Reality_DS::setLightIntensity( const QString& lightID, const float gain ) {
  RealitySceneData->setLightIntensity(lightID, gain);
}

QString getGeometryFileName( const DzNode* node, const QString defaultName ) {
  // Studio appends the number of vertices that an object has after
  // the name of the object if that object is a prop or a Poser-style
  // figure. The number of vertices is not part of the figure name if
  // it's a triaxis figure, coming from a DUF file.
  // On the other hand Reality needs to uniquely identify objects in
  // the whole runtime, regardless of the scene. Some products use 
  // ambiguos names, like "pants" or "bra" and so it's likely to have
  // collisions when those object names are used to identify ACSEL
  // shaders. 
  //
  // To make the product/object name unique we add the number of 
  // vertices all the time, even when it's not provided by Studio.
  // The same is done by the Poser plugin so that the same object is
  // identified in the same regardless of what application is used to
  // create the scene, Poser or DS.
  //

  QString fileName = node->getAssetId();
  // Find out if there is the number of vertices appended to the 
  // object name, as in FIG_12345
  QString numVerts;
  for (int i = fileName.length()-1; i >= 0; i--) {
    auto ch = fileName[i];
    if (ch >= '0' && ch <= '9') {
      numVerts.prepend(ch);
    }
    else {
      if (ch != '_') { 
        // Blanking the numVerts variable is used as a signal that 
        // the number of vertices was not found.
        numVerts.clear();
      }
      break;
    }
  }

  if (numVerts.isEmpty()) {
    // Collect information about the node's geometry
    DzObject* obj = node->getObject();
    if (obj) {
      DzShape* shape = obj->getCurrentShape();
      if (shape) {
        DzGeometry* geom = shape->getGeometry();
        int numVerts;
        if (!geom) {
          numVerts = 0;
        }
        else{
          numVerts = geom->getNumVertices();
        };
        fileName = QString("%1_%2").arg(fileName).arg(numVerts);
      }
    }
  }

  if ( fileName.isEmpty() ) {
    fileName = defaultName;
  }
  return fileName;
}

/**
 * Creates an object UID by taking the object name and adding a
 * unique hash. The hash is created by using a GUID and then shortening it
 * with a CRC function
 */
QString makeObjectUID( const DzNode* node ) {
  // QString GUID = ReGUID::getGUID(node);
  // if (GUID != "") {
  //   return GUID;
  // }
  QString GUID = ReGUID::getGUID();
  return QString("%1-%2")
    .arg(getGeometryFileName(node, node->getName()))
    .arg( crcFast(GUID.toUtf8().data(), GUID.count()) );
}

/**
 * Store the Reality GUID for a node in the node itself so that Studio
 * will store it in its scene file and then will make it available to 
 * Reality whenever needed
 */
void setGUIDForNode( DzNode* node, const QString& GUID ) {
  auto dataItem = qobject_cast<DzSimpleElementData*>(
    node->findDataItem(RE_DS_NODE_GUID)
  );
  // There is no previously attached data
  if (!dataItem) {
    dataItem = new DzSimpleElementData(RE_DS_NODE_GUID, true);
    node->addDataItem(dataItem);
  } 
  auto settings = dataItem->getSettings();
  settings->setStringValue(RE_DS_NODE_GUID, GUID);
  // We use also a private property because the "Fit to..." operation, and possibly
  // other Studio APIs, strip an object of the custom data once the conversion
  // is done. At this time we duplicate the data to avoid breaking compatibility
  // with the past versions of Reality which used the custom data.
  DzStringProperty* customProp = new DzStringProperty(RE_DS_NODE_GUID, true);
  customProp->setValue(GUID);
  node->addPrivateProperty(customProp);
}


void Reality_DS::addRealityObject( const QString& objID, 
                                   const DzNode* node ) {

  QString fileName = getGeometryFileName(node, objID);
  
  if ( !RealitySceneData->addObject(node->getLabel(), objID, fileName) ) {
    // if the addObject method return false it means that that object is already
    // in the database and we don't need to do anything else
    return;
  }
  
  if (node->getName().startsWith(RE_DS_UBER_LIGHT_PREFIX)) {
    auto reMat = RealitySceneData->getObject(objID);
    if (!reMat.isNull()) {
      reMat->markAsLight(true);
    }
  }

  // RE_LOG_INFO() << "Object " << objID << " added.";
  // If this is an instance then set the link between the instance
  // and its source object and get out of here.
  if (node->inherits("DzInstanceNode")) {
    DzNode* source = static_cast<const DzInstanceNode*>(node)->getTarget();
    QString srcGUID = ReGUID::getGUID(source);
    auto obj = RealitySceneData->getObject(objID);
    obj->setInstanceSourceID(srcGUID);
    realityIPC->objectAdded(objID);  
    return;
  }

  DzObject* obj = node->getObject();

  if (!obj) {
    RE_LOG_INFO() << "No object found! " << objID;
    return;
  }

  DzShape* shape = obj->getCurrentShape();
  if (!shape) {
    RE_LOG_INFO() << "Object has no shape!";
    return;
  }

  int numMats = shape->getNumAssemblyMaterials();

  // Make sure that we cache the ACSEL database operations so to not
  // slow-down the host
  // ReAcsel::getInstance()->startCaching();
  ReDSMaterialConverter* matConverter = ReDSMaterialConverter::getInstance();
  for ( int l = 0; l < numMats; l++) {
    DzMaterial* theMat = shape->getAssemblyMaterial(l);
    QVariantMap* matInfo = matConverter->convertMaterial(theMat);
    QString matName = theMat->getLabel();
    RealitySceneData->addMaterial(objID, matName, *matInfo);
    auto reMat = RealitySceneData->getMaterial(objID, matName);
    flatMatList.storeMaterial(theMat->getIndex(), reMat);
  }
  // ReAcsel::getInstance()->stopCaching();
  realityIPC->objectAdded(objID);  
};

void Reality_DS::shapeSwitchHandler() {
  RE_LOG_INFO() << "]! Shape switched!";
}

void setMonitorForMaterialProperties( const DzMaterial* mat, QObject* target) {
  auto i = mat->propertyListIterator();
  while( i.hasNext() ) {
    auto prop = i.next();
    // Get notifications when the material changes
    if (prop->isHidden() || prop->isAlias()) {
      continue; 
    }
    QObject::connect(prop, SIGNAL(currentValueChanged()), 
                     target, SLOT(watchMaterial()));
  }  
}

void Reality_DS::materialAdded( DzMaterial* dzMat ) {
  auto src = qobject_cast<DzShape*>(QObject::sender());
  if (!src) {
    RE_LOG_INFO() << "materialAdded() called is not a shape";
    return;
  }
  auto dzObj = qobject_cast<DzObject*>(src->getElementParent());
  if (!dzObj) {
    RE_LOG_INFO() << "Parent of shape instance is not a DzObject";
    return;
  }
  auto dzNode = qobject_cast<DzNode*>(dzObj->getElementParent());

  QString objID = ReGUID::getGUID(dzNode);
  auto matConverter = ReDSMaterialConverter::getInstance();
  auto matInfo = matConverter->convertMaterial(dzMat);
  RealitySceneData->addMaterial(objID, dzMat->getLabel(), *matInfo);
  realityIPC->objectDeleted(objID);
  realityIPC->objectAdded(objID);
  RE_LOG_INFO() << "Material added: " 
                << src->getName()
                << "/"
                << objID
                << "::"
                << dzMat->getName();
}

void Reality_DS::materialRemoved( DzMaterial* dzMat ) {
  auto src = qobject_cast<DzShape*>(QObject::sender());
  if (!src) {
    RE_LOG_INFO() << "materialAdded() called is not a shape";
    return;
  }
  RE_LOG_INFO() << "Material removed: " 
                << src->getName()
                << "/"
                << dzMat->getName()
                << " -- Nothing done!";
}

void Reality_DS::monitorCamera( const DzCamera* cam ) {
  connect(cam, SIGNAL(labelChanged(const QString&)), 
          this, SLOT(cameraRenamed(const QString&)));  
}

void Reality_DS::monitorLight( const DzLight* light ) {
  // Make sure that Reality has the current label for the light
  // This operation is actually required because lights being merged
  // by Studio might receive a different label than the one that was
  // stored in the Reality data saved with the scene
  RealitySceneData->renameLight(ReGUID::getGUID(light), light->getLabel());
  connect(light, SIGNAL(labelChanged(const QString&)),
          this, SLOT(lightRenamed(const QString&)));
  // Monitor the coordinates change
  connect(light, SIGNAL(transformChanged()), this, SLOT(updateLightPos()));
  // Monitor the change of properties
  auto i = light->propertyListIterator();
  while( i.hasNext() ) {
    auto prop = i.next();
    QString propName = prop->getName();
    if ( propName == RE_DS_LIGHT_INTENSITY    ||
         propName == RE_DS_LIGHT_SPREAD_ANGLE ||
         propName == RE_DS_LIGHT_COLOR        ||
         propName == RE_DS_LIGHT_ILLUMINATION)
    {
      connect( prop, SIGNAL(currentValueChanged()),
              this, SLOT(updateLightProperty()));
    }
  }
}


void Reality_DS::monitorNode( const DzNode* node ) {
  if (node) {
    if (isCamera(node)) {
      monitorCamera(static_cast<const DzCamera*>(node));
    }
    else if (isLight(node)) {
      monitorLight(static_cast<const DzLight*>(node));
    }
    else {
      // RE_LOG_INFO() << "Monitoring node " << node->getLabel();
      // Ask for notifications when the node's label is renamed
      connect(node, SIGNAL(labelChanged(const QString&)), 
              this, SLOT(nodeRenamed(const QString&)));
      // Ask for notifications when the node changes visibility
      connect(node, SIGNAL(visibilityChanged()), this, SLOT(visibilityChanged()));

      DzObject* obj = node->getObject();
      if (!obj) {
        // RE_LOG_INFO() << "No object found!";
        return;
      }
      DzShape* shape = obj->getCurrentShape();
      if (!shape) {
        // RE_LOG_INFO() << "Object has no shape!";
        return;
      }

      //! Monitor the geometry switching
      connect(obj, SIGNAL(currentShapeSwitched()), this, SLOT(shapeSwitchHandler()));

      // Get notifications for material-level changes
      connect(shape, SIGNAL(materialChanged()),this, SLOT(watchMaterial()));
      // When materials are replaced by a preset the materialListChanged()
      // signal is raised. In that case the old material is removed and a 
      // new material is instantiated. The materialRemoved() signal is not
      // raised in that occasion and so it cannot be used reliably.
      connect(shape, SIGNAL(materialListChanged()), this, SLOT(materialListChanged()));

      // Get notifications when a material is added or removed
      connect(shape, SIGNAL(materialAdded(DzMaterial*)), 
              this, SLOT(materialAdded(DzMaterial*)));
      connect(shape, SIGNAL(materialRemoved(DzMaterial*)), 
              this, SLOT(materialRemoved(DzMaterial*)));
      // Add a monitor for the material changes
      // ReDSMaterialListIterator i(matList);
      int numMats = shape->getNumAssemblyMaterials();

      QString objID = ReGUID::getGUID(node);
      for ( int l = 0; l < numMats; l++) {
        DzMaterial* mat = shape->getAssemblyMaterial(l);
        auto reMat = RealitySceneData->getMaterial(objID, mat->getName());
        if (!reMat.isNull()) {
          flatMatList.storeMaterial(mat->getIndex(), reMat);
        }
        // Get notifications when the material is selected
        connect(mat, SIGNAL(selectionStateChanged (DzMaterial*, bool)), 
                this, SLOT(materialSelected(DzMaterial*, bool)));
        // Scan the list of properties and monitor their changes
        setMonitorForMaterialProperties(mat, this);
      }
    }
  }
  else {
    RE_LOG_WARN() << "Node to be monitor is NULL";
  }
}

void Reality_DS::materialListChanged() {
  auto src = QObject::sender();
  if (!src->inherits("DzShape")) {
    return;
  }
  DzShape* shape = qobject_cast<DzShape*>(src);
  if (!shape) {
    return;
  }
  auto node = shape->getNode();

  int numMats = shape->getNumAssemblyMaterials();
  for ( int l = 0; l < numMats; l++) {
    DzMaterial* theMat = shape->getAssemblyMaterial(l);
    // Set the monitors for the new material because the old material
    // has been removed at this point. A new material instance has been
    // created by Studio.
    setMonitorForMaterialProperties(theMat, this);
    QString objID = ReGUID::getGUID(node);
    QString key = QString("%1::%2").arg(objID).arg(theMat->getName());
    auto reObj = RealitySceneData->getObject(objID);
    if (!reObj.isNull()) {
      auto reMat = reObj->getMaterial(theMat->getName());
      if (!reMat.isNull()) {
        flatMatList.storeMaterial(theMat->getIndex(), reMat);
      }
    }
    materialRefreshTable[key] = theMat->getIndex();
  }  
}

