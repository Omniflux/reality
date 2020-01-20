/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_SCENEEXPORTER_H
#define RE_QT_SCENEEXPORTER_H

#include "ReDefs.h"
#include "ReBaseSceneExporter.h"

namespace Reality {

class RE_LIB_ACCESS ReQtSceneExporter : public ReBaseSceneExporter {

public:
  ReQtSceneExporter( ReSceneData* scene ) : ReBaseSceneExporter(scene) {
  };
 ~ReQtSceneExporter() {
  };

  void exportScene( const int frameNo, boost::any& sceneData );

  void prepare() {
    
  }

  void cleanup() {
    
  }
};


}

#endif
