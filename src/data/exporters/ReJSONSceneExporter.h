/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef ReJSONSceneExporter_h
#define ReJSONSceneExporter_h

#include <boost/any.hpp>

#include "reality_lib_export.h"
#include "ReBaseSceneExporter.h"

namespace Reality {
  class ReSceneData;
}


namespace Reality {

/**
 * Exports a Reality scene to JSON. It was used in the past to pass the data
 * to Poser but it's not used anymore. It has been kept because it could turn
 * useful in the future. The conversion is done by first using the QVariantMap
 * format and then converting that to JSON. 
 */
class REALITY_LIB_EXPORT ReJSONSceneExporter : public ReBaseSceneExporter {

public:
  // Constructor: ReJSONSceneExporter
  ReJSONSceneExporter( ReSceneData* scene ) : ReBaseSceneExporter(scene) {
  };
  // Destructor: ReJSONSceneExporter
 ~ReJSONSceneExporter() {
  };

  void exportScene( const int frameNo, boost::any& sceneData );

  void prepare() {
    
  }

  void cleanup() {
    
  }
};


}

#endif
