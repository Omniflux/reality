/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReQtTextureImporterFactory.h"
#include "ReBandImporter.h"
#include "ReBricksImporter.h"
#include "ReCheckersImporter.h"
#include "ReCloudsImporter.h"
#include "ReColorMathImporter.h"
#include "ReConstantImporter.h"
#include "ReDistortedNoiseImporter.h"
#include "ReFBMImporter.h"
// #include "ReFresnelImporter.h"
#include "ReGrayscaleImporter.h"
#include "ReImageMapImporter.h"
// #include "ReInvertedMapImporter.h"
#include "ReMarbleImporter.h"
#include "ReWoodImporter.h"
#include "ReMathImporter.h"
#include "ReMixTextureImporter.h"
// #include "ReMultiMixImporter.h"
// #include "ReWrinkleImporter.h"

namespace Reality {
  
QHash<int, ReQtTextureImporterPtr> ReQtTextureImporterFactory::importers;

ReQtTextureImporterPtr ReQtTextureImporterFactory::getImporterService( const int texType ) {
  if (!importers.contains(texType)) {
    switch(texType) {
      case TexImageMap:
        importers[texType] = ReQtTextureImporterPtr(new ReQtImageMapImporter());
        break;
      case TexMath:
        importers[texType] = ReQtTextureImporterPtr(new ReQtMathImporter());
        break;
      case TexColorMath:
        importers[texType] = ReQtTextureImporterPtr(new ReQtColorMathImporter());
        break;
      case TexBand:
        importers[texType] = ReQtTextureImporterPtr(new ReQtBandImporter());
        break;
      case TexBricks:
        importers[texType] = ReQtTextureImporterPtr(new ReQtBricksImporter());
        break;
      case TexCheckers:
        importers[texType] = ReQtTextureImporterPtr(new ReQtCheckersImporter());
        break;
      case TexClouds:
        importers[texType] = ReQtTextureImporterPtr(new ReQtCloudsImporter());
        break;
      case TexConstant:
        importers[texType] = ReQtTextureImporterPtr(new ReQtConstantImporter());
        break;
      case TexFBM:
        importers[texType] = ReQtTextureImporterPtr(new ReQtFBMImporter());
        break;
      // case TexFresnelColor:
      //   importers[texType] = ReQtTextureImporterPtr(new ReQtFresnelImporter());
      //   break;
      case TexMarble:
        importers[texType] = ReQtTextureImporterPtr(new ReQtMarbleImporter());
        break;
      case TexMix:
        importers[texType] = ReQtTextureImporterPtr(new ReQtMixTextureImporter());
        break;
      // case TexMultiMix:
      //   importers[texType] = ReQtTextureImporterPtr(new ReQtMultiMixImporter());
      //   break;
      case TexDistortedNoise:
        importers[texType] = ReQtTextureImporterPtr(new ReQtDistortedNoiseImporter());
        break;
     case TexWood:
       importers[texType] = ReQtTextureImporterPtr(new ReQtWoodImporter());
       break;
      // case TexInvertMap:
      //   importers[texType] = ReQtTextureImporterPtr(new ReQtInvertedMapImporter());
      //   break;
      case TexGrayscale:
        importers[texType] = ReQtTextureImporterPtr(new ReQtGrayscaleImporter());
        break;       
    }
  }
  return importers[texType];
}

ReQtTextureImporterPtr ReQtTextureImporterFactory::getImporter( const ReTextureType texType ) {
  return getImporterService( texType );
}

}