/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReQtMaterialImporterFactory.h"

#include "ReClothImporter.h"
#include "ReGlassImporter.h"
#include "ReGlossyImporter.h"
#include "ReLightMaterialImporter.h"
#include "ReMatteImporter.h"
#include "ReMetalImporter.h"
#include "ReMirrorImporter.h"
#include "ReNullImporter.h"
#include "ReSkinImporter.h"
#include "ReVelvetImporter.h"
#include "ReWaterImporter.h"


namespace Reality {
  
QHash<int, ReQtMaterialImporterPtr> ReQtMaterialImporterFactory::importers;

ReQtMaterialImporterPtr ReQtMaterialImporterFactory::getImporterService( const ReMaterialType matType ) {
  if (!importers.contains(matType)) {
    switch(matType) {
      case MatCloth:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtClothImporter());
        break;
      case MatGlossy:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtGlossyImporter());
        break;
      case MatSkin:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtSkinImporter());
        break;
      case MatMatte:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtMatteImporter());
        break;
      case MatMix:
        // importers[matType] = ReQtMaterialImporterPtr(new ReQtMixImporter());
        break;
      case MatGlass:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtGlassImporter());
        break;
      case MatWater:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtWaterImporter());
        break;
      case MatMetal:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtMetalImporter());
        break;
      case MatMirror:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtMirrorImporter());
        break;
      case MatVelvet:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtVelvetImporter());
        break;
      case MatHair:
        // importers[matType] = ReQtMaterialImporterPtr(new ReQtHairImporter());
        break;
      case MatNull:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtNullImporter());
        break;
      case MatLight:
        importers[matType] = ReQtMaterialImporterPtr(new ReQtLightMaterialImporter());
        break; 
      default:
        break;       
    }
  }
  return importers[matType];
}

ReQtMaterialImporterPtr ReQtMaterialImporterFactory::getImporter( const ReMaterialType matType ) {
  return getImporterService(matType);
}

}