/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#include "ReQtMaterialExporterFactory.h"

#include "ReClothExporter.h"
#include "ReGlassExporter.h"
#include "ReGlossyExporter.h"
#include "ReLightMaterialExporter.h"
#include "ReMatteExporter.h"
#include "ReMetalExporter.h"
#include "ReMirrorExporter.h"
 // #include "ReMixExporter.h"
#include "ReNullExporter.h"
#include "ReSkinExporter.h"
#include "ReVelvetExporter.h"
#include "ReWaterExporter.h"


namespace Reality {
  
QHash<int, ReQtMaterialExporterPtr> ReQtMaterialExporterFactory::exporters;

ReQtMaterialExporterPtr 
ReQtMaterialExporterFactory::getExporterService( const int matType ) {
  if (!exporters.contains(matType)) {
    switch(matType) {
      case MatCloth:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtClothExporter());
        break;
      case MatGlossy:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtGlossyExporter());
        break;
      case MatSkin:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtSkinExporter());
        break;
      case MatMatte:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtMatteExporter());
        break;
      // case MatMix:
      //   exporters[matType] = ReQtMaterialExporterPtr(new ReQtMixExporter());
      //   break;
      case MatGlass:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtGlassExporter());
        break;
      case MatWater:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtWaterExporter());
        break;
      case MatMetal:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtMetalExporter());
        break;
      case MatMirror:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtMirrorExporter());
        break;
      case MatVelvet:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtVelvetExporter());
        break;
      case MatNull:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtNullExporter());
        break;
      case MatLight:
        exporters[matType] = ReQtMaterialExporterPtr(new ReQtLightMaterialExporter());
        break;        
    }
  }
  return exporters[matType];
}

ReQtMaterialExporterPtr ReQtMaterialExporterFactory::getExporter( const ReMaterial* mat ) {
  return getExporterService(mat->getType());
}

}