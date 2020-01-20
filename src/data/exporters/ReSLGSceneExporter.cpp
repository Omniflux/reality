/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReSLGSceneExporter.h"

namespace Reality {
  
void ReSLGSceneExporter::exportScene( const int frameNo, boost::any& sceneData ) {
  sceneData = QVariant("SLG Scene");
}

void ReSLGSceneExporter::cleanup() {

}

}