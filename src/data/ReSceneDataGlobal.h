/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RESCENDATA_GLOBAL_H
#define RESCENDATA_GLOBAL_H

#include "reality_lib_export.h"

namespace Reality {
  class ReSceneData;
}


//! Access to the global ReSceneData pointer 
extern "C" REALITY_LIB_EXPORT Reality::ReSceneData* RealitySceneData;

#endif
