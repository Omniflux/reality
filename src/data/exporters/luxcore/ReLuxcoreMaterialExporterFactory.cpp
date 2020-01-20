/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "exporters/luxcore/ReLuxcoreMaterialExporterFactory.h"
#include "exporters/luxcore/ReMatteExporter.h"
#include "exporters/luxcore/ReGlassExporter.h"
#include "exporters/luxcore/ReGlossyExporter.h"
#include "exporters/luxcore/ReMetalExporter.h"
#include "exporters/luxcore/ReClothExporter.h"
#include "exporters/lux/ReMirrorExporter.h"
#include "exporters/lux/ReMixExporter.h"
#include "exporters/lux/ReNullExporter.h"
#include "exporters/lux/ReStatueExporter.h"
#include "exporters/lux/ReLightMaterialLuxExporter.h"

namespace Reality {
  
ExportersMap ReLuxcoreMaterialExporterFactory::exporters;

void ReLuxcoreMaterialExporterFactory::clearExporterCache() {
  QHashIterator<int, ReMaterialExporterPtr> i(exporters);
  while( i.hasNext() ) {
    i.next();
    ReMaterialExporterPtr val = i.value();
    val.clear();
  }
  exporters.clear();
}

ReMaterialExporterPtr ReLuxcoreMaterialExporterFactory::getExporterService( const int matType ) {
  bool renderAsStatue = RealitySceneData->isRenderAsStatueEnabled();
  if (renderAsStatue) {
    if (!exporters.contains(matType)) {
      if (matType != MatNull && matType != MatLight && matType != MatMix) {
        exporters[matType] = ReMaterialExporterPtr(new ReStatueLuxExporter());
        return exporters[matType];
      }
    }  
  }
  if (!exporters.contains(matType)) {
    switch(matType) {
      case MatCloth:
        exporters[matType] = ReMaterialExporterPtr(new ReClothLuxcoreExporter());
        break;
      case MatSkin:
      case MatGlossy:
        exporters[matType] = ReMaterialExporterPtr(new ReGlossyLuxcoreExporter());
        break;
      case MatMatte:
        exporters[matType] = ReMaterialExporterPtr(new ReMatteLuxcoreExporter());
        break;
      case MatMix:
        exporters[matType] = ReMaterialExporterPtr(new ReMixLuxExporter());
        break;
      case MatGlass:
        exporters[matType] = ReMaterialExporterPtr(new ReGlassLuxcoreExporter());
        break;
      case MatWater:
        exporters[matType] = ReMaterialExporterPtr(new ReGlassLuxcoreExporter());
        break;
      case MatMetal:
        exporters[matType] = ReMaterialExporterPtr(new ReMetalLuxcoreExporter());
        break;
      case MatMirror:
        exporters[matType] = ReMaterialExporterPtr(new ReMirrorLuxExporter());
        break;
      case MatVelvet:
        exporters[matType] = ReMaterialExporterPtr(new ReMatteLuxcoreExporter());
        break;
      // case MatHair:
      //   exporters[matType] = ReLuxMaterialExporterPtr(new ReHairLuxExporter());
      //   break;
      case MatNull:
        exporters[matType] = ReMaterialExporterPtr(new ReNullLuxExporter());
        break;
      case MatLight:
        exporters[matType] = ReMaterialExporterPtr(new ReLightMaterialLuxExporter());
        break;        
    }
    
  }
  return exporters[matType];
}

ReMaterialExporterPtr ReLuxcoreMaterialExporterFactory::getExporter( const ReMaterial* mat ) {
  return getExporterService(mat->getType());
}

}