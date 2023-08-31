/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLuxTextureExporterFactory.h"

#include "ReBandExporter.h"
#include "ReBricksExporter.h"
#include "ReCheckersExporter.h"
#include "ReCloudsExporter.h"
#include "ReColorMathExporter.h"
#include "ReConstantExporter.h"
#include "ReDistortedNoiseExporter.h"
#include "ReFBMExporter.h"
#include "ReFresnelExporter.h"
#include "ReGrayscaleExporter.h"
#include "ReImageMapExporter.h"
#include "ReInvertedMapExporter.h"
#include "ReMarbleExporter.h"
#include "ReMathExporter.h"
#include "ReMixTextureExporter.h"
#include "ReMultiMixExporter.h"
#include "ReTexture.h"
#include "ReWoodExporter.h"
//#include "ReWrinkleExporter.h"


namespace Reality {
  
QHash<int, ReLuxTextureExporterPtr> ReLuxTextureExporterFactory::exporters;

ReLuxTextureExporterPtr ReLuxTextureExporterFactory::getExporterService( const int expType ) {
  if (!exporters.contains(expType)) {
    switch(expType) {
      case TexImageMap:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxImageMapExporter());
        break;
      case TexMath:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxMathExporter());
        break;
      case TexColorMath:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxColorMathExporter());
        break;
      case TexBand:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxBandExporter());
        break;
      case TexBricks:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxBricksExporter());
        break;
      case TexCheckers:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxCheckersExporter());
        break;
      case TexClouds:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxCloudsExporter());
        break;
      case TexConstant:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxConstantExporter());
        break;
      case TexFBM:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxFBMExporter());
        break;
      case TexFresnelColor:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxFresnelExporter());
        break;
      case TexMarble:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxMarbleExporter());
        break;
      case TexMix:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxMixExporter());
        break;
      case TexMultiMix:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxMultiMixExporter());
        break;
      case TexDistortedNoise:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxDistortedNoiseExporter());
        break;
     case TexWood:
       exporters[expType] = ReLuxTextureExporterPtr(new ReLuxWoodExporter());
       break;
      case TexInvertMap:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxInvertedMapExporter());
        break;
      case TexGrayscale:
        exporters[expType] = ReLuxTextureExporterPtr(new ReLuxGrayscaleExporter());
        break;       
    }
  }
  return exporters[expType];
}

ReLuxTextureExporterPtr ReLuxTextureExporterFactory::getExporter( const ReTexturePtr tex ) {
  return getExporterService(tex->getType());
}

}