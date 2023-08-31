/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "exporters/lux/ReLuxMaterialExporterFactory.h"

#include "ReMaterial.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "exporters/lux/ReStatueExporter.h"

#include "exporters/lux/ReClothExporter.h"
#include "exporters/lux/ReGlassExporter.h"
#include "exporters/lux/ReGlossyExporter.h"
//#include "exporters/lux/ReHairExporter.h"
#include "exporters/lux/ReLightMaterialLuxExporter.h"
#include "exporters/lux/ReMatteExporter.h"
#include "exporters/lux/ReMetalExporter.h"
#include "exporters/lux/ReMirrorExporter.h"
#include "exporters/lux/ReMixExporter.h"
#include "exporters/lux/ReNullExporter.h"
#include "exporters/lux/ReSkinExporter.h"
#include "exporters/lux/ReVelvetExporter.h"
#include "exporters/lux/ReWaterExporter.h"


namespace Reality {
  
QHash<int, ReLuxMaterialExporterPtr> ReLuxMaterialExporterFactory::exporters;

void ReLuxMaterialExporterFactory::clearExporterCache() {
  QHashIterator<int, ReLuxMaterialExporterPtr> i(exporters);
  while( i.hasNext() ) {
    i.next();
    ReLuxMaterialExporterPtr val = i.value();
    val.clear();
  }
  exporters.clear();
}

ReLuxMaterialExporterPtr ReLuxMaterialExporterFactory::getExporterService( const int matType ) {
  bool renderAsStatue = RealitySceneData->isRenderAsStatueEnabled();
  if (renderAsStatue) {
    if (!exporters.contains(matType)) {
      if (matType != MatNull && 
          matType != MatLight && 
          matType != MatMix &&
          matType != MatGlass ) {
        exporters[matType] = ReLuxMaterialExporterPtr(new ReStatueLuxExporter());
        return exporters[matType];
      }
    }  
  }
  if (!exporters.contains(matType)) {
    switch(matType) {
      case MatCloth:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReClothLuxExporter());
        break;
      case MatGlossy:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReGlossyLuxExporter());
        break;
      case MatSkin:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReSkinLuxExporter());
        break;
      case MatMatte:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReMatteLuxExporter());
        break;
      case MatMix:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReMixLuxExporter());
        break;
      case MatGlass:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReGlassLuxExporter());
        break;
      case MatWater:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReWaterLuxExporter());
        break;
      case MatMetal:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReMetalLuxExporter());
        break;
      case MatMirror:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReMirrorLuxExporter());
        break;
      case MatVelvet:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReVelvetLuxExporter());
        break;
      // case MatHair:
      //   exporters[matType] = ReLuxMaterialExporterPtr(new ReHairLuxExporter());
      //   break;
      case MatNull:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReNullLuxExporter());
        break;
      case MatLight:
        exporters[matType] = ReLuxMaterialExporterPtr(new ReLightMaterialLuxExporter());
        break;        
    }
  }
  return exporters[matType];
}

ReLuxMaterialExporterPtr ReLuxMaterialExporterFactory::getExporter( const ReMaterial* mat ) {
  return getExporterService(mat->getType());
}

}