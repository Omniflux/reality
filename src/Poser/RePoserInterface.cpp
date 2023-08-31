/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#include "RePoserInterface.h"

#include <QVariant>
#include <QSettings>

#include "RealityBase.h"
#include "RealityRunner.h"
#include "ReIPC.h"
#include "ReLogger.h"
#include "ReLuxRunner.h"
#include "RePythonTools.h"
#include "RePythonSceneExporter.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"


using namespace python;
using namespace Reality;

/**
 * RePoserBase methods
 */
void RePoserBase::setLibraryPaths( const python::list& pyLibs ) {
  QVariantList libs;
  convertPythonList(pyLibs, libs);
  RealityBase::setLibraryPaths(QVariant(libs).toStringList());
}

void RePoserBase::closeGUI() {
  RE_LOG_INFO() << "Closing the GUI...";
  realityIPC->closeGUI();
  RE_LOG_INFO() << "GUI closed!";
}

QByteArray exchangeBuffer;
//! IPC services start and stop functions to be called by the
//! host-side plug-in
void RePoserBase::startHostSideServices( const HostAppID appID ) {
  RealityBase::getRealityBase()->startHostSideServices(appID);
}

void RePoserBase::stopHostSideServices() {
  RealityBase::getRealityBase()->stopHostSideServices();
}

int RePoserBase::runGUI( const QString& directory ) {
  return static_cast<int>(
    RealityRunner::getInstance()->launchGUI(directory)
  );
}

void RePoserBase::objectAdded( const QString& objectName ) {
  realityIPC->objectAdded(objectName);
}

void RePoserBase::objectRenamed( const QString& objectID, QString& newName ) {
  realityIPC->objectRenamed(objectID, newName);
}

void RePoserBase::objectDeleted( const QString& objName ) {
  realityIPC->objectDeleted(objName);
}

//! Called when a material is selected in the 
//! host app. Reality then selectes the material in the 
//! material editor.
void RePoserBase::materialSelected( const QString& objName, const QString& matID ) {
  realityIPC->materialSelected(objName, matID);
}

void RePoserBase::lightAdded( const QString& lightID ) {
  realityIPC->lightAdded(lightID);
}

void RePoserBase::lightDeleted( const QString& lightID ) {
  realityIPC->lightDeleted(lightID);
}

void RePoserBase::lightRenamed( const QString& objectID, QString& newName ) {
  realityIPC->lightRenamed(objectID, newName);
}

void RePoserBase::lightUpdated( const QString& lightID ) {
  realityIPC->lightUpdated(lightID);
}

void RePoserBase::cameraAdded( const QString& cameraID ) {
  realityIPC->cameraAdded(cameraID);
}

void RePoserBase::cameraRenamed( const QString& objectID, 
                                 const QString& newName ) 
{
  realityIPC->cameraRenamed(objectID, newName);
}

void RePoserBase::cameraDataChanged( const QString& cameraID ) {
  realityIPC->cameraDataChanged(cameraID);
}

void RePoserBase::hostCameraSelected( const QString& cameraID ) {
  realityIPC->hostCameraSelected(cameraID);
}

char const* RePoserBase::commandStackPop() {
  exchangeBuffer = RealityBase::getRealityBase()->commandStackPop().toAscii();
  return exchangeBuffer.data();
}

int RePoserBase::getNumCommands() {
  return RealityBase::getRealityBase()->getNumCommands();
}

void RePoserBase::setHostAppID( const QString& appID ) {
  RealityBase::getRealityBase()->setHostAppID(appID);
}

void RePoserBase::setNewScene() {
  realityIPC->setNewScene();
}

void RePoserBase::runGuiLux( const QString& sceneFileName, const uint maxThreads ) {
  ReLuxRunner lr;
  lr.runGuiLux( sceneFileName, 
                maxThreads, 
                RealitySceneData->getLuxLogLevel() );
}

void RePoserBase::renderLuxQueue( const python::list& pyRenderQueue, 
                                  const uint maxThreads ) 
{
  QVariantList renderQueue;
  convertPythonList(pyRenderQueue, renderQueue);
  ReLuxRunner lr;
  lr.runGuiLux( QVariant(renderQueue).toStringList(), 
                maxThreads,
                RealitySceneData->getLuxLogLevel() );
}

int RePoserBase::findHostAppID( const QString& hostAppNameStr ) {
  return static_cast<int>(
           RealityBase::getRealityBase()->findHostAppID(
              hostAppNameStr
           )
         );
}

void RePoserBase::exportStarted( int numObjects ) {
  realityIPC->exportStarted(numObjects);
}

void RePoserBase::exportingObject( const QString& objName ) {
  realityIPC->exportingObject(objName);
}

void RePoserBase::exportFinished() {
  realityIPC->exportFinished();
}

//! This function is called to alert the user that the dimensions for the render are
//! not set. 
void RePoserBase::renderDimensionsNotSet() {
  realityIPC->renderDimensionsNotSet();
}

char const * RePoserBase::getLibraryVersion() {
  return RealityBase::getRealityBase()->getLibraryVersion();
}

// void RePoserBase::writeToLog( const char* msg ) {
//   RE_LOG_INFO() << msg;
// }

void RePoserBase::writeToLog( const QString& msg ) {
  RE_LOG_INFO() << msg.toStdString();
}


void RePoserBase::setHostVersion( const QString& msg ) {
  RealityBase::getRealityBase()->setHostVersion(msg);
}

void RePoserBase::pauseMaterialPreview( const bool pause ) {
  if (pause) {
    realityIPC->GUIPauseMatPreview();
  }
  else {
    realityIPC->GUIResumeMatPreview();
  }
}

python::str RePoserBase::getConfigValue( const QString& cfgKey ) {
  python::str str = RealityBase::getConfiguration()->value(cfgKey)
                      .toString().toUtf8().data();
  return str;
}

void RePoserBase::startACSELCaching() {
  RealityBase::getRealityBase()->startACSELCaching();
};

void RePoserBase::stopACSELCaching() {
  RealityBase::getRealityBase()->stopACSELCaching();  
};

python::str RePoserBase::sanitizeMaterialName( const QString& matName ) {
  python::str str = ::sanitizeMaterialName(matName).toUtf8().data();
  return str;
}

void RePoserBase::updateAnimationLimits( const int startFrame, 
                                         const int endFrame, 
                                         const int fps) {
  realityIPC->updateAnimationLimits(startFrame, endFrame, fps);
}


/**
 * RePoserSceneData methods
 */

//! Initialization of static member
QByteArray RePoserSceneData::tempCharData;

void RePoserSceneData::addCamera( python::dict& camData ) {
  QVariantMap data;
  convertPythonDict(camData, data);
  RealitySceneData->addCamera(data);
}

void RePoserSceneData::updateCamera( const QString& camID, const python::dict& camData ) {
  QVariantMap data;
  convertPythonDict(camData, data);
  RealitySceneData->updateCamera(camID, data);
}

void RePoserSceneData::addMaterial( const QString& objID, 
                                    const QString& matID,
                                    const python::dict& pyMatData ) 
{
  QVariantMap matData;
  convertPythonDict(pyMatData, matData);
  RealitySceneData->addMaterial( objID, matID, matData );
}

void RePoserSceneData::updateMaterial( const QString& objID, 
                                       const QString& matID,
                                       const python::dict& pyMatData ) 
{
  QVariantMap matData;
  convertPythonDict(pyMatData, matData);
  RealitySceneData->updateMaterial( objID, 
                                    matID, 
                                    matData );
}

void RePoserSceneData::changeMaterialType( const QString& objID, 
                                           const QString& matID, 
                                           const Reality::ReMaterialType newType,
                                           const python::dict& pyMatData )
{
  QVariantMap matData;
  convertPythonDict(pyMatData, matData);
  RealitySceneData->changeMaterialType( 
      objID, 
      matID,
      newType,
      matData
  );  
}

python::str RePoserSceneData::getSceneFileName( int frameNo ) {
  QString sceneName = RealitySceneData->getSceneFileName();
  python::str fileName = expandFrameNumber(sceneName, frameNo).toUtf8().data();
  return fileName;
}

void RePoserSceneData::restoreScene( const python::dict& pyData ) {
  QVariantMap sceneData;
  convertPythonDict(pyData, sceneData);
  RealitySceneData->restoreScene(sceneData);
}

void RePoserSceneData::addLight( const QString& lightID, 
                                 const python::dict& pyData ) 
{
  QVariantMap lightData;
  convertPythonDict(pyData, lightData);
  RealitySceneData->addLight(lightID, lightData);
}

void RePoserSceneData::setLightMatrix( const QString& lightID, 
                                       const python::tuple& pyData ) 
{
  QVariantList matrix;
  convertPythonList(pyData, matrix);
  RealitySceneData->setLightMatrix(lightID, matrix);
}

void RePoserSceneData::updateLight( const QString& lightID, 
                                    const python::dict& pyData ) 
{
  QVariantMap lightData;
  convertPythonDict(pyData, lightData);
  RealitySceneData->updateLight(lightID, lightData);
}

const char* RePoserSceneData::renderScene( const QString& format, int frameNo ) {
  RE_LOG_INFO() << "Rendering Poser scene...";
  tempCharData = RealitySceneData->exportScene(format, frameNo).toUtf8();
  return tempCharData.data();  
}

python::dict RePoserSceneData::exportScene()  {
  RePythonSceneExporter exporter;
  boost::any data;
  exporter.exportScene(0, data);
  return boost::any_cast<python::dict>(data);
}

void RePoserSceneData::copyVertexData( const python::list& vertexList, 
                                       const python::list& normalList ) 
{
  int listLen = python::len(vertexList);
  float* verts = RealitySceneData->getGeometryVertexBuffer();
  float* norms = RealitySceneData->getGeometryNormalBuffer();

  // The list is a setof X, Y, Z values. We scan each group of 3 
  for (int i = 0; i < listLen; i += 3) {
    // The vertices are stored continuguosly in a flat array. The sequence is
    // formatted as X, Y, Z
    verts[i]   = python::extract<float>(vertexList[i]);
    verts[i+1] = python::extract<float>(vertexList[i+1]);
    verts[i+2] = python::extract<float>(vertexList[i+2]);

    norms[i]   = python::extract<float>(normalList[i]);
    norms[i+1] = python::extract<float>(normalList[i+1]);
    norms[i+2] = python::extract<float>(normalList[i+2]);
  }
};

void RePoserSceneData::copyUVData( const python::list uvList,
                                   const python::list& vertexList,
                                   const python::list& normalList  ) 
{
  ReUVPoint* UVs = RealitySceneData->getGeometryUVPointBuffer();
  float* verts = RealitySceneData->getGeometryVertexBuffer();
  float* norms = RealitySceneData->getGeometryNormalBuffer();

  int listLen = python::len(uvList);
  for (int i = 0; i < listLen; i += 3) {
    // The UV data in Pyhton is a list that contains a series of 
    // three values: an integer which is the vertex index and
    // two floats, the value for U and the value for V
    int vertexIndex = python::extract<int>(uvList[i]);
    UVs[vertexIndex][0] = python::extract<float>(uvList[i+1]);
    UVs[vertexIndex][1] = python::extract<float>(uvList[i+2]);

    // The vertices are stored continuguosly in a flat array. The sequence is
    // formatted as X, Y, Z
    verts[vertexIndex*3]   = python::extract<float>(vertexList[vertexIndex*3]);
    verts[vertexIndex*3+1] = python::extract<float>(vertexList[vertexIndex*3+1]);
    verts[vertexIndex*3+2] = python::extract<float>(vertexList[vertexIndex*3+2]);

    norms[vertexIndex*3]   = python::extract<float>(normalList[vertexIndex*3]);
    norms[vertexIndex*3+1] = python::extract<float>(normalList[vertexIndex*3+1]);
    norms[vertexIndex*3+2] = python::extract<float>(normalList[vertexIndex*3+2]);
  }  
};


void RePoserSceneData::copyPolygonData( const python::list polyList ) {
  int* faces = RealitySceneData->getGeometryFaceBuffer();
  int listLen = python::len(polyList);
  for (int i = 0; i < listLen; i += 3) {
    // The polygons are stored as a list of tuples holding the A,B,C
    // vertex indices for each triangle
    faces[i]   = python::extract<int>(polyList[i]);
    faces[i+1] = python::extract<int>(polyList[i+1]);
    faces[i+2] = python::extract<int>(polyList[i+2]);
  }
};
