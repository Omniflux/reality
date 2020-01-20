/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#include "ReSceneDataGlobal.h"
#include "ReSceneData.h"
#include "ReRenderContext.h"
#include "ReGeometryObject.h"

using namespace Reality;

ReRenderContext* ReRenderContext::instance = NULL;

ReRenderContext* ReRenderContext::getInstance() {
  if (!instance) {
    instance = new ReRenderContext();
  }
  return instance;
}

void ReRenderContext::init() {
  hasIBLFlag = hasSunFlag = usesOCLFlag = false;

  sceneFileName = RealitySceneData->getSceneFileName();
  imageFileName = RealitySceneData->getImageFileName();
  
  ReGeometryObjectIterator i(RealitySceneData->getObjects());
  objectInstatiators.clear();
  hasInstancesFlag = false;

  while(i.hasNext()) {
    i.next();
    ReGeometryObjectPtr obj = i.value();
    if (obj.isNull()) {
      RE_LOG_INFO() << "NULL OBJ!!!!! " << i.key();
    }
    if (!obj.isNull() && obj->isInstance()) {
      hasInstancesFlag = true;
      objectInstatiators << obj->getInstanceSourceID();
    }
  }

  numMeshLights = numSpotLights = numInfiniteLights = 0;

  ReLightIterator li(RealitySceneData->getLights());
  while(li.hasNext()) {
    li.next();
    auto light = li.value();
    if (!light->isLightOn()) {
      continue;
    }

    switch (light->getType()) {
      case SunLight:
        hasSunFlag = true;
        break;
      case IBLLight:
        hasIBLFlag = true;
        break;
      case MeshLight:
        numMeshLights++;
        break;
      case SpotLight:
        numSpotLights++;
        break;
      case InfiniteLight:
        numInfiniteLights++;
        break;

      default:
        break;
    }
  }

  usesOCLFlag = RealitySceneData->isOCLRenderingON();
  usesLuxCoreFlag = RealitySceneData->cpuAccelerationEnabled() || usesOCLFlag;
}
