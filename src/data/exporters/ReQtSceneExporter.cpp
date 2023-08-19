/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReVersion.h"
#include "ReQtSceneExporter.h"
#include "qt/ReLightExporter.h"
#include "qt/ReGeometryObjectExporter.h"
#include "qt/ReCameraExporter.h"
#include "qt/ReQtMaterialExporterFactory.h"
#include "qt/ReVolumeExporter.h"
#include "ReGeometryObject.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"

using namespace Reality;

void ReQtSceneExporter::exportScene( const int /*frameNo*/, 
                                     boost::any& sceneData  ) 
{    
  QVariantMap objects, 
              materials,
              lights,
              cameras,
              volumes,
              sceneMap;
  

  ReQtGeometryObjectExporter objExporter;
  // Get the list of all the objects in the scene and iterate
  // through them exporting all the materials
  ReGeometryObjectDictionary objs = scene->getObjects();
  ReGeometryObjectIterator i(objs);
  // Export all objects
  while( i.hasNext() ) {
    i.next();
    ReGeometryObjectPtr obj = i.value();

    objects[obj->getInternalName()] = objExporter.exportGeometryObject(obj);
  }

  // Export the lights
  ReQtLightExporter lightExporter;
  ReLightIterator li(scene->getLights());
  while( li.hasNext() ) {
    li.next();
    lights[li.key()] = lightExporter.exportLight(li.value());
  }

  // Export the cameras
  ReQtCameraExporter cameraExporter;
  ReCameraIterator ci(*scene->getCameras());
  while( ci.hasNext() ) {
    ci.next();
    cameras[ci.key()] = cameraExporter.exportCamera(ci.value());
  }

  // Export the volumes
  ReQtVolumeExporter volExporter;
  ReVolumeIterator vi(scene->getVolumes());
  while( vi.hasNext() ) {
    vi.next();
    volumes[vi.key()] = volExporter.exportVolume(vi.value());
  }

  QVariantMap sceneParameters;
  RealitySceneData->getOutputData(sceneParameters);
  // When saving the data embedded in a scene it's important to not save the 
  // number of threads, so that the scene can be loaded into another machine
  // and have the number of threads be calculated for the current hardware.  
  sceneParameters[KEY_SCENE_NUM_THREADS] = 0;
  
  // Put all the elements together
  sceneMap[KEY_SCENE_MAIN_VERSION] = REALITY_MAIN_VERSION;
  sceneMap[KEY_SCENE_SUB_VERSION]  = REALITY_SUB_VERSION;
  sceneMap[KEY_SCENE_PATCH_VERSION]= REALITY_PATCH_VERSION;
  sceneMap["scene"]                = sceneParameters;
  sceneMap["objects"]              = objects;
  sceneMap["lights"]               = lights;
  sceneMap["cameras"]              = cameras;
  sceneMap["volumes"]              = volumes;

  sceneData = sceneMap;
}
