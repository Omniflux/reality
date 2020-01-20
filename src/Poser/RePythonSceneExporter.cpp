/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#include "ReSceneDataGlobal.h"
#include "RePythonSceneExporter.h"
#include "RePythonTools.h"
#include <boost/python.hpp>

#ifdef __clang__
// Avoid being harassed about missing switch cases and other thing.
#pragma clang diagnostic ignored "-Wswitch"
#endif

namespace python = boost::python;

using namespace Reality;

void RePythonSceneExporter::exportScene( const int frameNo, 
                                         boost::any& data ) 
{  
  try {
    python::dict pySceneData;
    data = pySceneData;
    auto exporter = RealitySceneData->getSceneExporter("map");
    boost::any sceneData;
    // No matter what is passed in the frameNo parameter, we specify
    // zero because we are not rendering but exporting the data for saving
    exporter->exportScene(0, sceneData);
    QVariantMap qSceneData = boost::any_cast<QVariantMap>(sceneData);
    QVariantMapToDict(qSceneData, pySceneData);
  }
  catch(...) {
    RE_LOG_INFO() << "Error in conversion of scene data";
  }
} 


