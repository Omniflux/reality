/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#ifndef RE_POSER_INTERFACE_H
#define RE_POSER_INTERFACE_H

#include <boost/python.hpp>

#include "ReDefs.h"
#include "ReLightMaterial.h"
#include "ReLogger.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReSceneResources.h"
#include "ReTools.h"


namespace python = boost::python;

using namespace Reality;

//! This class provides the Poser interface to the \ref RealityBase class.
class RePoserBase {
public:
  static void cameraAdded( const QString& cameraID );
  static void cameraDataChanged( const QString& cameraID );
  static void cameraRenamed( const QString& objectID, 
                             const QString& newName );
  static void hostCameraSelected( const QString& cameraID );

  static void closeGUI();
  //! Called when a material is selected in the 
  //! host app. Reality then selectes the material in the 
  //! material editor.
  static char const* commandStackPop();
  static void exportFinished();
  static void exportingObject( const QString& objName );
  static void exportStarted( int numObjects );
  static int findHostAppID( const QString& hostAppNameStr );
  static char const* getLibraryVersion();

  static int runGUI( const QString& directory );
  static void objectAdded( const QString& objectName );
  static void objectDeleted( const QString& objName );
  static void objectRenamed( const QString& objectID, QString& newName );
  static void startHostSideServices( const HostAppID appID = Poser );  
  static void stopHostSideServices();

  static int getNumCommands();
  static void lightAdded( const QString& lightID);
  static void lightDeleted( const QString& lightID);
  static void lightRenamed( const QString& objectID, QString& newName);
  static void lightUpdated( const QString& lightID );
  static void materialSelected(const QString& objName, const QString& matID);
  static void renderLuxQueue( const python::list& pyRenderQueue, const uint maxThreads );
  static void runGuiLux( const QString& sceneFileName, const uint maxThreads );
  static void setHostAppID( const QString& appID );
  static void setNewScene();
  static void pauseMaterialPreview( const bool pause );
  //! This function is called to alert the user that the dimensions for the render are
  //! not set. 
  static void renderDimensionsNotSet();
  static void setHostVersion( const QString& msg );

  //! Saves the library paths sent by the host-app.
  //! The input is a JSON-formatted string listing
  //! an array of directory names
  static void setLibraryPaths( const python::list& pyLibs );
  static void writeToLog( const QString& msg );

  static python::str getConfigValue( const QString& cfgKey );

  //! Used to start the caching of the ACSEL database. This is used when
  //! importing materials form Poser. The caching minimizes the impact
  //! of SQL queries on the performance of Poser
  static void startACSELCaching();

  //! Used to stop the caching of the ACSEL database. This is used when
  //! importing materials form Poser. The caching minimizes the impact
  //! of SQL queries on the performance of Poser
  static void stopACSELCaching();

  static python::str sanitizeMaterialName( const QString& matName );

  static void updateAnimationLimits( const int startFrame, const int endFrame, const int fps);
};

class RePoserSceneData {
private:
  //! Buffer used to return characters from Python inteface functions
  static QByteArray tempCharData;

public:
  RePoserSceneData() {};
  ~RePoserSceneData() {};

  // Objects
  inline bool addObject( const QString& objName, 
                         const QString& internalName, 
                         const QString& geometryName) 
  {
    return RealitySceneData->addObject(objName, internalName, geometryName);
  }

  inline void renameObject( const QString& objectID, const QString& newName) {
    RealitySceneData->renameObject(objectID, newName);
  }

  inline void renameObjectID( const QString objectID, const QString newID) {
    RealitySceneData->renameObjectID(objectID, newID);
  }

  inline bool hasObject( const QString& objectID ) {
    return RealitySceneData->hasObject(objectID);
  }

  inline void deleteObject( const QString& objectID, const bool usePublicName = false ) {
    RealitySceneData->deleteObject(objectID, usePublicName);
  }

  inline const char* getObjectName( const QString& objectID ) {
    return RealitySceneData->getObjectName(objectID);
  }

  inline void markObjectAsLight( const QString objName, const bool onOff ) {
    RealitySceneData->markObjectAsLight(objName, onOff);
  }

  inline const python::list getObjectIDs() {
    QStringList l = RealitySceneData->getObjectIDs();
    python::list t;
    int count = l.count();
    for (int i = 0; i < count; i++) {
      t.append(l[i].toStdString());
    }
    return t;
  }

  inline const char* getMaterialShortType(const QString& objID, const QString& matName) {
    return RealitySceneData->getMaterialShortType(objID, matName).toAscii();
  }

  inline const char* findObjectByName( const QString& objName ) {
    return RealitySceneData->findObjectByName(objName).toUtf8();
  }

  inline void markObjectsForDeletion() {
    RealitySceneData->markObjectsForDeletion();
  }
  
  inline bool clearObjectDeletionFlag( const QString& objectID ) {
    return RealitySceneData->clearObjectDeletionFlag(objectID);
  }

  inline void deleteOrphanedObjects() {
    RealitySceneData->deleteOrphanedObjects();
  }

  inline void setObjectVisible( const QString& objectID, const bool yesNo ) {
    RealitySceneData->setObjectVisible(objectID, yesNo);
  }

  inline int getNumObjects() {
    return RealitySceneData->getObjects().count();
  }

  // Materials
  inline quint16 getNumMaterials() {
    return RealitySceneData->getNumMaterials();
  }

  inline void restoreMaterial( const QString& objName,
                               const QString& matID,
                               const QString& materialData ) {
    RealitySceneData->restoreMaterial(objName, matID, materialData);
  }

  inline python::list getMaterialNames( const QString& objectID ) {
    QStringList matList = RealitySceneData->getMaterialNames(objectID);
    python::list l;
    int count = matList.count();
    for (int i = 0; i < count; i++) {
      l.append(matList[i].toStdString());
    }
    return l;
  }

  /*
   * Cameras
   */
  void addCamera( python::dict& camData );
  void updateCamera( const QString& camID, const python::dict& camData );
  inline void renameCamera( const QString& cameraID, const QString& newName ) {
    RealitySceneData->renameCamera(cameraID, newName);
  }

  inline void removeCamera( const QString camID ) {
    RealitySceneData->removeCamera(camID);
  }

  inline void restoreCamera( const QString& dataStr ) {
    RealitySceneData->restoreCamera(dataStr);
  }

  inline void selectCamera( const QString& dataStr ) {
    RealitySceneData->selectCamera(dataStr);
  }

  //! If the matID passed identifies a light material, then this 
  //! method returns the id of the associated light
  inline python::str getLightIDFromMaterial( const QString& objID, 
                                             const QString& matID ) 
  {
    auto mat = RealitySceneData->getMaterial(objID, matID);
    if (!mat) {
      return "";
    }
    if (!(mat->getType() == MatLight)) {
      return "";
    }

    python::str str = mat.staticCast<ReLightMaterial>()->getLight()->getID().toUtf8().data();

    return str;
  }

  /*
   * Lights
   */
  void addLight( const QString& lightID, const python::dict& pyData );
  void setLightMatrix( const QString& lightID, const python::tuple& pyData );
  void updateLight( const QString& lightID, const python::dict& pyData );

  inline void renameLight( const QString& lightID, const QString& newName ) {
    RealitySceneData->renameLight(lightID, newName);
  }

  inline void deleteLight( const QString& lightID ) {
    RealitySceneData->deleteLight(lightID);
  }

  inline void setLightType( const QString& lightID, const int newType ) {
    ReLightType newLightType = static_cast<ReLightType>(newType);
    RealitySceneData->setLightType(lightID, newLightType);
  }

  inline void restoreLight( const QString& dataStr ) {
    RealitySceneData->restoreLight(dataStr);
  }

  inline void setIBLRotation( const float angle ) {
    RealitySceneData->setIBLRotation(angle);
  }

  inline void setIBLImage( const QString& sIBLFileName ) {
    RealitySceneData->setIBLImage(sIBLFileName);
  }

  inline void setLightIntensity( const QString& lightID, const float gain ) {
    RealitySceneData->setLightIntensity(lightID, gain);
  }

  inline void setIsHostAreaLight( const QString& lightID, const bool flag ) {
    auto light = RealitySceneData->getLight(lightID);
    if (!light) {
      RE_LOG_INFO() << "Light " << lightID << " was not found";
      auto lights = RealitySceneData->getLights();
      foreach( ReLightPtr light, lights) {
        RE_LOG_INFO() << "  ]] " << light->getID();
      }
    }
    if (light && light->getType() == MeshLight) {
      light->setIsHostAreaLight(flag);
    }
  }

  inline void removeAllLights() {
    RealitySceneData->removeAllLights();    
  }

  inline void backupLights() { 
    RealitySceneData->backupLights();
  }

  inline void restoreLightsFromBackup() { 
    RealitySceneData->restoreLightsFromBackup();
  }

  /*
   * Materials
   */
  void addMaterial( const QString& objID, 
                    const QString& matID,
                    const python::dict& pyMatData ); 
  void updateMaterial( const QString& objID, 
                       const QString& matID,
                       const python::dict& pyMatData ); 
  void changeMaterialType( const QString& objectID, 
                           const QString& materialID, 
                           const Reality::ReMaterialType newType,
                           const python::dict& pyMatData );

  /*
   * Scene handling
   */
  //! Returns the file name for a given frame. Frames can have their
  //! sequence number as part of the files name
  python::str getSceneFileName( int frameNo = -1 );

  void restoreScene( const python::dict& pyData );

  const char* renderScene( const QString& format, int frameNo );
  python::dict exportScene();

  inline int getWidth() {
    return RealitySceneData->getWidth();
  }

  inline int getHeight() {
    return RealitySceneData->getHeight();
  }

  inline void newScene() {
    RealitySceneData->newScene();
  }

  inline void setFrameSize( const unsigned int width, 
                            const unsigned int height) 
  {
    RealitySceneData->setFrameSize(width, height);
  }

  inline int getNumThreads() {
    return RealitySceneData->getNumThreads();
  }

  inline bool needsSaving() {
    return RealitySceneData->doesNeedsSaving();
  }

  inline bool isMaterialVisible( const QString& objectID, 
                                 const QString& matName ) 
  {
    ReMaterialPtr mat = RealitySceneData->getMaterial(
                          objectID, 
                          matName
                        );
    if (mat.isNull()) {
      return false;
    }
    return mat->isVisibleInRender();
  }

  /*
   * Geometry
   */
  inline void newGeometryBuffer( const QString& bufferName,
                                 const int numVertices, 
                                 const int numTriangles, 
                                 const bool hasUVs ) const
  {
    RealitySceneData->newGeometryBuffer(bufferName, 
                                        numVertices,
                                        numTriangles, 
                                        hasUVs);
  }

  void copyVertexData( const python::list& vertexList, 
                       const python::list& normalList );

  void copyUVData( const python::list uvData,
                   const python::list& vertexList,
                   const python::list& normalList  );

  void copyPolygonData( const python::list polyData );

  inline void renderSceneStart( const QString& sceneFileName, int frameNo ) {
    RealitySceneData->renderSceneStart(sceneFileName, frameNo);
  }

  inline void renderSceneExportMaterial( const QString& materialName, 
                                         const QString& objectName, 
                                         const QString& shapeName ) 
  {
    RealitySceneData->renderSceneExportMaterial(
      materialName, 
      objectName, 
      shapeName, 
      Reality::Poser
    );
  }

  inline void renderSceneFinish( const bool runRenderer ) {
    RealitySceneData->renderSceneFinish(runRenderer);
  }

  inline void renderSceneObjectBegin( const QString& objName ) {
    RealitySceneData->renderSceneObjectBegin(objName);
  }

  inline void renderSceneObjectEnd( const QString& objName ) {
    RealitySceneData->renderSceneObjectEnd(objName);
  }

  inline void renderSceneCustomData( const QString& str ) {
    RealitySceneData->renderSceneIncludeFileCustomData(str);
  }

  inline python::str getSceneResourceObjectPath() {
    python::str str = ReSceneResources::getInstance()->getObjectsPath().toUtf8().data();
    return str;
  }

  inline python::str getSceneResourceRelativePath( const QString& p ) {
    python::str str = ReSceneResources::getInstance()->getRelativePath(p).toUtf8().data();
    return str;
  }

  inline const python::list getRenderCameraPosition() {
    ReCameraPtr cam = RealitySceneData->getSelectedCamera();

    ReVector position;
    const ReMatrix* matrix = cam->getMatrix();
    matrix->getPosition(position);
    vectorToLux(position);

    python::list t;
    t.append(convertUnit(position.X, Poser));
    t.append(convertUnit(position.Y, Poser));
    t.append(convertUnit(position.Z, Poser));
    return t;
  }
};


#endif
