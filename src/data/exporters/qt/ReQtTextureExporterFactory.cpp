/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReQtTextureExporterFactory.h"
#include "ReBandExporter.h"
#include "ReBricksExporter.h"
#include "ReCheckersExporter.h"
#include "ReCloudsExporter.h"
#include "ReColorMathExporter.h"
#include "ReConstantExporter.h"
#include "ReDistortedNoiseExporter.h"
#include "ReFBMExporter.h"
#include "ReGrayscaleExporter.h"
#include "ReImageMapExporter.h"
#include "ReMarbleExporter.h"
#include "ReWoodExporter.h"
#include "ReMathExporter.h"
#include "ReMixTextureExporter.h"

namespace Reality {
  
QHash<int, ReQtTextureExporterPtr> ReQtTextureExporterFactory::exporters;

ReQtTextureExporterPtr ReQtTextureExporterFactory::getExporterService( const int expType ) {
  if (!exporters.contains(expType)) {
    switch(expType) {
      case TexImageMap:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtImageMapExporter());
        break;
      case TexMath:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtMathExporter());
        break;
      case TexColorMath:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtColorMathExporter());
        break;
      case TexBand:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtBandExporter());
        break;
      case TexBricks:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtBricksExporter());
        break;
      case TexCheckers:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtCheckersExporter());
        break;
      case TexClouds:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtCloudsExporter());
        break;
      case TexConstant:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtConstantExporter());
        break;
      case TexFBM:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtFBMExporter());
        break;
      case TexMarble:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtMarbleExporter());
        break;
      case TexMix:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtMixTextureExporter());
        break;
      case TexDistortedNoise:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtDistortedNoiseExporter());
        break;
     case TexWood:
       exporters[expType] = ReQtTextureExporterPtr(new ReQtWoodExporter());
       break;
      case TexGrayscale:
        exporters[expType] = ReQtTextureExporterPtr(new ReQtGrayscaleExporter());
        break;       
    }
  }
  return exporters[expType];
}

ReQtTextureExporterPtr ReQtTextureExporterFactory::getExporter( const ReTexturePtr tex ) {
  return getExporterService(tex->getType());
}

}