/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef ReSLGSceneExporter_H
#define ReSLGSceneExporter_H

#include "ReDefs.h"
#include "ReBaseSceneExporter.h"

namespace Reality {

/*
  Class: ReSLGSceneExporter
 */

class RE_LIB_ACCESS ReSLGSceneExporter : public ReBaseSceneExporter {
private:

  QString sceneStr; 
public:
  // Constructor: ReSLGSceneExporter
  ReSLGSceneExporter( ReSceneData* scene ) : ReBaseSceneExporter(scene) {
  };
  // Destructor: ReSLGSceneExporter
 ~ReSLGSceneExporter() {
  };

  void exportScene( const int frameNo, boost::any& sceneData );

  void prepare() {
    
  }
  void cleanup();
};


}

#endif
