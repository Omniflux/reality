/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef REALITYPANEL_H
#define REALITYPANEL_H

#include <QWidget>
#include <QDebug>
#include <QSharedPointer>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QLibrary>

#include "ReIPC.h"
#include "ReUiContainer.h"
#include "RealityDataRelay.h"
#include "ReSceneDataModel.h"
#include "ReTextureEditor.h"
#include "ReTextureChannelDataModel.h"
#include "ui_realitypanel.h"
#include "ReLightEditor.h"
#include "zmq.hpp"
#include "ReExportProgressDialog.h"
#include "ReTextureEditorAdapter.h"

namespace Ui {
  class RealityPanel;
}

using namespace Reality;

// Typedef for our list of tab info
typedef QHash<QString,unsigned short int> ReMatPageList;

/**
 * This is the main GUI of Reality. It is defined as a Widget so that it can be easily re-parented.
 */
class RealityPanel : public QWidget, public Ui::RealityPanel {
  Q_OBJECT
  
public:

  //! Constructor: RealityPanel
  explicit RealityPanel(QWidget *parent = 0, QString ipaddress = "localhost");

  // Destructor: ~RealityPanel
  ~RealityPanel();

  //! Method: setServerAddress
  //!   Sets the ip address for the server. This is used to connect to a
  //!   host-side instance of Reality that is not running on the same machine
  //!   of the GUI. Experimental feature. 2012-10-23.
  void setServerAddress( const QString serverAddress );

  //! Closes the IPC and disconnected from the host-app side
  void disconnect();
  
private:

  //! The data model that we use for the TreeView that lists all the materials
  //! found in the scene
  ReSceneDataModel* sceneDataModel;

  //! We want our data sorted by clicking on the header of the TreeView so we
  //! use a QSortFilterProxyModel object to make this happen.
  QSortFilterProxyModel* sortedSceneDataModel;

  //! Pointer to the currently selected object. It saves a lot of un-necessary calls 
  //! to methods that would retrieve this value.
  //! Set by <materialSelected()>
  ReGeometryObject* currentObject;

  //! Pointer to the currently selected material. It saves a lot of un-necessary calls 
  //! to methods that would retrieve this value.
  //! Set by <materialSelected()>
  ReMaterial* currentMaterial;

  //! The material editor current active.
  ReMaterialEditor* currentMatEditor;

  QModelIndex currentIndex;

  //! Data model for editing textures attached to a material  
  ReTextureChannelDataModelPtr textureChannelDataModel;

  //! Adapter to control the editing of the alpha map texture
  ReTextureEditorAdapterPtr alphaAdapter;

  void init();

  void initAdvancedTab();

  //! Method: setMaterialContextMenu
  //!   Creates the right-click menu for the material list
  void setMaterialContextMenu();

  //! Returns how many active lights are in the scene.
  int numOfActiveLights();

  //! Used to signal that we are updating the UI. For example, when the
  //! user selects a new material.
  bool updatingUI;

  //! The Lights Editor
  ReLightEditor* reLightEditor;

  //! Pointer to the material preview maker
  ReMaterialPreview* previewMaker;

  //! The address of the host-add side of Reality. It defaults to "localhost" and
  //! normally it never needs to be changed. Experimental feature. 2012-12-23.
  QString serverAddress;

  /**
   * From time to time it is necessary to pause the material preview during
   * material updates. There are actually two situations that can cause the
   * preview to be paused. A global "turn off" even, for example an update
   * of many materials from a single operation, like when the user loads
   * a material preset. In that case a single material file contains 
   * the definition for multiple materials and those materials are applied in
   * quick succession. There is no point in turning off/on the preview for
   * every material, it's best to shut it off for the whole operation and 
   * then turn it on after the file is done loading.
   * We call this situation a "global event"
   *
   * Then there are situations when a method needs to turn off the preview,
   * do some local change and then turn it on again. That is a "local event".
   *
   * The material preview is updated if both the global and local flags
   * are enabled. The global flag takes precedence over the local one. In
   * other words, if the local flag is true and the global one is false then
   * the preview will not happen.
   */
  bool globalPreviewPaused;
  bool localPreviewPaused;
  
  // Contextual menu for the material list
  QMenu* matContexMenu;

  // Contextual menu Actions
  QAction* toClothAction;
  QAction* toGlassAction;
  QAction* toGlossyAction;
  QAction* toMatteAction;
  QAction* toMetalAction;
  QAction* toMirrorAction;
  QAction* toNullAction;
  QAction* toSkinAction;
  QAction* toVelvetAction;
  QAction* toWaterAction;
  QAction* toLightAction;

  //! ACSEL save preset action
  QAction* acselSaveAction;

  //! ACSEL delete shader action
  QAction* acselDeleteShaderAction;

  //! Save as Universal Shader
  QAction* uShaderSaveAction;

  //! Apply a Universa Shader
  QAction* uShaderApplyAction;

  //! Material synchronization action
  QAction* syncMatsAction;
  //! Revert material to original action
  QAction* revertToOriginalAction;

  //! The export progress dialog
  ReExportProgressDialog* exportProgressDlg;

  //! A timer to constantly ping the host-app side every second and let it know
  //! that the GUI is still running
  QTimer* pingTimer;

  //! This timer is used to start a material preview after a material has
  //! be selected for a suffieintly long time. This technique is used to 
  //! avoid starting a new preview for a material that the user has selected
  //! only in passing.
  QTimer* matPreviewTimer;

  //! Convenience pointer to the parent of this widget cast to the right
  //! type to avoid doing the same cast over and over again.
  ReUiContainer* container;

  /**
   * Shows the data about the currently selected object.
   */
  void showObjectInformation();

  /**
   * Detetrmines if the texture editor needs to be visible or not,
   * baed on the tab number passed in the first parameter, which is
   * expected to tbe the current tab number.
   */
  void setTextureEditorVisibility();

  /**
   Connects a newly created material editor signals to the handlers
   that manage the connection of the material editor to the texture 
   editor and other key components of the UI.

   IMPORTANT: 
   Both the Material Editor and the Texture Editor share pointers to the textures.
   The M.E. uses textures for channels like Kd, Ks etc. The textures are shown via
   the \ref ReTextureAvatar class. 

   When the user edits a texture she clicks on the texture avatar and that causes the
   texture to be passed to the Texture Editor. At this point the same pointer to a texture 
   is shared by both the M.E. and the T.E. and ideally they would show the same version
   of the texture at any time. 

   It turns out that this is easily done by simply emitting any signal from any of the 
   widgets in the Texture Editor. For example the Color Picker. When a signal is emitted 
   Qt repaints all the widgets. Since we share the same pointer then both the M.E. and 
   the T.E. paint the same version. 

   To make this work it's important that each custom widget has its own paintEvent()
   method defined.

   So, we don't need to do anything at this level to repaint the M.E. when the user
   updates the textures, it happens automatically.
  */  
  void addMatEditorSignals( ReMaterialEditor* editor );

  template<typename T> void makeNewEditor();

  /**
   * Lookup table to identify a shape for material preview based on the
   * figure type and material name. It's organized as follows:
   *   <figure alias:QString> => 
   *       <material name:QString> => <shape name:QString>
   *
   * For example:
   *   DAZ4G: 
   *     1_SkinFace: face
   *     1_Lip: lips
   *     5_Iris: eye
   *     5_Sclera: eye
   *     5_Pupil: eye
   *
   *  Genesis:
   *     1_SkinFace: face
   *     1_Lip: lips
   *     5_Iris: eye
   *     5_Sclera: eye
   *     5_Pupil: eye
   *
   *  Genesis2:
   *     Face: face
   *     Lips: lips
   *     5_Iris: eye
   *     5_Sclera: eye
   *     5_Pupil: eye
   * 
   * This table is loaded from the resource named 
   * textResources/previewShapes.json
   */
  QVariantMap previewShapes;

  void loadPreviewShapes();
  QString useShapePreview( const QString& matName, const QString& geomName );

  //! Finds if a volume has been saved to ACSEL by looking at all the materials
  //! that are using this shader. The material names are then used to query
  //! the ACSEL shader set and find out if they have a volume ID already set
  //! \param objectID The ID of the object that contains the materials 
  //!                 linked to the volume.
  //! \param setID The ID of the shader set that contains the materials to 
  //!              be queried
  //! \param volumeID The name of the volume to be found
  //! \return The ID of the volume, if found, 0 otherwise
  int findShaderVolumeInAcsel( const QString& objectID, 
                               const QString& setID, 
                               const QString volumeID );

  //! Finds what ACSEL shader set is needed to save the selected materials
  //! Possible scenarios:
  //!
  //! - All selected materials have a set assigned previously. This
  //!   is the simplest situation. The assigned shader set is returned.
  //!
  //! - Multiple materials are selected and some belong to a set and 
  //!   some belong to another. This is a mistake and the error is reported.
  //!   
  //! - Some materials belong to a set and others are not assigned. The set
  //!   returned is the one found. All materials are saved into that set.
  //!
  //! - No selected materials are found to be assigned to any
  //!   set. The whole group of materials for the object is searched and, 
  //!   if a set is found, it is returned as the set to be assigned.
  //!
  //! - The selected materials don't belong to any set but there 
  //!   are materials that are assigned to a set and others that are
  //!   assigned to another set. In this case the user is asked to
  //!   select which shader set has to be used.
  //!
  //! \param shaderSet. Output. The name of the shader set to use. If no
  //!                   shader set is found, or if the generic shader set is
  //!                   found the string is returned empty.
  //!                     
  //! \return  If an error or other condition is found, which
  //!          should stop further elaboration, then the function returns false
  bool findAcselShaderSet( QString& shaderSet );

  //! Performs some sanity check before a group of selected materials
  //! is updated by paste or other actions that are supposed to work
  //! on multiple materials.
  //! 
  //! \param matType The selected materials will be checked to be of 
  //!                the same type of this parameter. If they are not, an
  //!                error message is shown and the returned set will be 
  //!                empty.
  //! \return On successful run the method will return a list of selected
  //!         indices corresponding to the selected materials.
  QList<QModelIndex> checkMaterialsBeforeUpdate( ReMaterialType matType );

  //! Disconnects the Qt signals before closing the program. This is
  //! necessary in order to avoid receiving signals during that 
  //! critical phase, triggered by some object being deleted.
  void disconnectSignals();

private slots:

  //! This slot pings the host-app side to let it know that the GUI is still running.
  void pingHost();

  void matExpandAll();
  void matCollapseAll();

  // Method: syncMaterialSelection
  //   Called to synchronize the selection of material with the host-app
  //   Selecting a material in Poser, for example, sends a signal to Reality
  //   and Reality tries to select the same material in its editor
  void syncMaterialSelection(const QString& objectID, const QString& matID);

  // Save the state of the material editor splitter
  void saveSplitteState(int,int);

  // Right-click menu for the material list
  void materialsMenuRequested(const QPoint & pos);  

  // //!   Responds to the signal launched when the user wants to create a 
  // //!   new alpha map texture.
  // void addNewAlphaTexture( const QString& ID, const ReTextureType texType );

  // //! Responds to the signal launched when the user wants to link an existing
  // //! texture to the alpha channel
  // void linkAlphaMapTexture( const QString& ID, const QString& textureName);

  // //! Responds to the signal raised when the user unlinks a texture from the alpha channel
  // void unlinkAlphaMapTexture( const QString& ID );

  //! Send to the host-app side a request to create the same
  //! texture that has been just created in the GUI
  void makeNewTexture( const QString& objectID, 
                       const QString& materialID, 
                       const QString& channelName, 
                       const ReTextureType textureType,
                       const ReTexture::ReTextureDataType dtype );

  //! Send the signal to the host-app side to create a new texture
  //! that is linked to a master texture. This can happen when
  //! textures like Mix or Bricks need to create subtextures for their
  //! channels.
  void makeNewSubTexture( const QString& objectID, 
                          const QString& materialName, 
                          const QString& channelID,
                          const QString& masterTextureName,
                          const QString& newTextureName,
                          const ReTextureType texType,
                          const ReTexture::ReTextureDataType texdataType );


  //! Notify the host-side that a texture has been linked to a channel
  void linkTexture( const QString& objectID, 
                    const QString& materialID, 
                    const QString& channelName, 
                    const QString& textureName );

  //! Notify the host-side that a sub-texture has been linked to a master textrue
  void linkSubTexture( const QString& objectID, 
                       const QString& materialID, 
                       const QString& channelName, 
                       const QString& textureName,
                       const QString& subTextureName );
  void unlinkSubTexture( const QString& objectID,
                         const QString& materialID,
                         const QString& textureName,
                         const QString& channelName );
   
  //! Notify the host-side that a texture has been unlinked from a channel
  void unlinkTexture( const QString& objectID, 
                      const QString& materialID, 
                      const QString& channelName );

  //! Notify the host-side that a texture needs to be replaced with data
  //! copied from another texture.
  void replaceHostTexture( const QString& channelID,
                           const QString& jsonTextureData,
                           const QString& masterTextureName = "" );

  //! Transfers the lights data from the host-app side
  void loadLights();

  //! Save what was the last tab selected in the main tabs widget
  void saveMainTabsConfig(int);

  //! Connects to the thread that provides the material previews
  void connectToPreviewMaker();

  void setSelectedCamera(const QString cameraID);

  //! Refresh the material editor.
  void refreshMaterialEditor();

  void materialTypeChanged( const QString objectID, 
                            const QString materialName, 
                            const quint16 newTypeInt ); 

  //! Converts a material that was converted to a light back to its
  //! former material definition
  void lightToMaterial( const QString objectID,
                        const QString materialName, 
                        const QString lightID );

  // Communicates to the host-app side that the texture type has conaged.
  void updateServerTextureType(const QString&, const ReTextureType);  
  
  /**
   * Updates the value for Reality's configration from the Advanced tab.
   */
  void updateConfiguration( int newVal ); 
  /**
   * Updates the value for Reality's configration from the Advanced tab.
   */
  void updateConfiguration( bool newVal );

  //! Provides information to the user about the status of the export process
  void exportStarted(int);
  void exportingObject(const QString);
  void exportFinished();

  void renderDimensionsNotSet();

  //! Enforces the policy that an item must be always selected in the list of
  //! objects/materials
  void checkObjectSelection( const QItemSelection&, const QItemSelection&);

  //! When the data from the host is received this method selectes the 
  //! first item in the model.
  void selectFirstItem();

  //! Opens a File|Open dialog box to search for a directory to use for 
  //! storeing the lxs and png files when creating a new scene
  void browseDefaultFolder();

  //! Sets the new default location for scene files
  void updateDefaultLocation(const QString& newVal);

  void pauseMaterialPreview( bool pause );

  void pauseMaterialPreviewGlobally( bool pause );

  bool isMaterialPreviewEnabled();

  //! Save the volumes connected to an ACSEL or Universal preset.
  int savePresetVolume(const QString& objectID, 
                       const QString& setID, 
                       const QString& volumeName);

  //! Save the selected materials as ACSEL Shaders
  void saveACSELShaders();

  //! Deletes an ACSEL shader from the set
  void deleteACSELShaders();
  
  //! Save the selected Universal Shader
  void saveUniversalShader();

  //! Apply a Universal Shader
  void applyUniversalShader();

  //! Synchronize materials to a base one. At this time it only works for
  //! Glossy and Skin.
  void synchronizeMaterials( const ReMaterial* baseMat, 
                             QList<ReMaterial*>& toBeSynchronized );

  //! Revert the shader to the original definition stored, before it was
  //! converted or changed by the user
  void revertShaderToOriginal();

  //! This is the method that responds to the scheduled material preview
  void scheduledMatPreviewHandler();

public slots:

  //! Starts the transfer of all the objects for the scene data from
  //! the host-app to the GUI
  void requestDataFromHostApp();

  //! Support for the Copy/Paste of materials
  void copyMaterial();
  void pasteMaterial( const ReQtMaterialImporter::ReplaceTextureMode = ReQtMaterialImporter::Keep );
  //! Thsi verison simply calls pasteMaterial() with the parameter set to true
  void pasteMaterialWithTextures();

  /**
   This method is called by the "upAndRunning" signal sent by the <RealityDataRelay> object
   , which is a QThread, when the thread started running its main loop. This method loads
   the scene data from the host-side portion of Reality.
   */
  void fetchData();

  // Signal handlers
  void materialSelected(const QModelIndex& index);
  void cameraAdded(const QString cameraID);
  void cameraRemoved(const QString cameraID);
  void cameraRenamed(const QString objectID, const QString newName);

  // Light handling
  void lightAdded(const QString lightID);

  void lightRenamed(const QString lightID, const QString newName);

  //! Host application identified itself
  void setWindowTitle();

  void objectAdded(const QString);
  void objectDeleted(const QString);

  //! Called when a figure/object is renamed in the host app scene while
  //! Reality is running
  void objectRenamed(const QString oldName, const QString newName);

  //! Called when a figure/object has changed internal name in the host app scene while
  //! Reality is running
  void objectIDRenamed(const QString objectID, const QString newID);

  //! Called when an object changed visibility in the host app
  void objectVisibilityChanged(const QString objectID, const bool isVisible);

  void materialUpdated(const QString materialName, const QString objectID);

  //! Texture editing
  void editTexture( ReTexturePtr tex, QString& channelID, const bool isRoot = true );

  void previousBreadcrumb( ReTexturePtr tex );
  
  //! The texture has been updated by the user and needs to be updated in the server side
  void updateServerTexture(ReTexturePtr texture, const QString& paramName, const QVariant& value);

  //! The materials has been updated by the user and needs to be updated in the server side
  void updateServerMaterial( QString objectID, QString materialName, QString valueName, QVariant value );

  //! The light has been updated by the user and needs to be updated in the server side
  void updateServerLight( const QString lightID, const QString paramName, const QVariant value );

  //! The camera has been updated by the user and needs to be updated in the server side
  void updateServerCamera( const QString cameraID, const QString paramName, const QVariant value );

  //! Updates the bitmap that shows the material preview.
  void updatePreview(QString matName, QString previewID, QImage* preview);

  //! Performs all the tasks needed to show the material updated in the material editor
  //! and it flags the material as modified in the material list.
  void refreshMaterial( ReMaterial* mat = 0, const bool refreshPreview = true );

  void changeMaterialType( int newType );

  //! Schedule the material preview to be started. This method resets the
  //! matPrevieTimer timer for a few milliseconds. If the timer expires 
  //! the the material preview is started.
  void scheduleMatPreview();

  //! Updates the material preview
  void refreshMatPreview( const bool forceRefresh = true );

  //! Verifies that the conditions for rendering are correct.
  //! \return True if rendering can be performed, flase otherwise.
  bool renderPreFlightCheck();

  //! Rendering
  void renderFrame();

  void renderAnimation(int startFrame, int endFrame);
  
  //! Save Scene in the host-app
  void saveScene();

  //! Refresh the Scene with the data from the host-ap
  void refreshScene();

  //! Load the volumes
  void loadVolumes();

  void volumeAdded(const QString& volName);
  void volumeDeleted(const QString& volName);
  void volumeRenamed(const QString& oldName, const QString& newName);
  void volumeUpdated(const QString& volName);

  //! Scene reset
  void setNewScene();

  //! Synchronize scene data from GUI to host-app side
  void syncSceneData();

  //! Load the output data received from the host-app side into the GUI side
  //! Parameters:
  //!   - newData: A JSON-formatted string with the new values.
  void showOutputData();

  //! This function asks the user to select a shader set that be applied
  //! to a series of selected materials. The idea is to use a compatible
  //! shader set for a new figure. For example, to apply a V4 shader set
  //! to M4, to save time in defining customer shader set
  void acselApplyCompatibleShaderSet();

signals:
  void closed();
};


#endif // REALITYPANEL_H
