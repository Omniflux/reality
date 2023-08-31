/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReJSONSceneExporter.h"

#include <boost/any.hpp>
#include <QVariantMap>
#include <QJson/Serializer>

#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"


using namespace Reality;

/**
 The internal organization of the data structure, before being
 converted to JSON, is as follows:

 ROOT("RealityScene") = {
   "version" : String,
   "objects" : {
      "objectName1" : {
        "visible" : Boolean,
        "isLight" : Boolean,
        "name" : String,
        "internalName" : String,
        "geometryFile" : String,
        "materials" : {
          "mat1" : MaterialData,
          .
          .
          .
          "matN" : ...
        }      
      }
      .
      .
      .
      "objectNameN" : ...
   },
   "lights" : {   
      .
      .
      .
   },
   "cameras" : {   
      .
      .
      .
   }
 }
 */

void ReJSONSceneExporter::exportScene( const int /*frameNo*/, 
                                       boost::any& sceneData  ) 
{
  ReBaseSceneExporter* exporter = RealitySceneData->getSceneExporter("map");
  boost::any mapData;
  exporter->exportScene(0, mapData);

  QJson::Serializer json;
  QString sceneStr = json.serialize(boost::any_cast<QVariantMap>(mapData));

  sceneData = sceneStr;
}
