/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_METAL_LUXCORE_EXPORTER_H
#define RE_METAL_LUXCORE_EXPORTER_H

#include "reality_lib_export.h"
#include "ReMetal.h"
#include "exporters/lux/ReLuxMaterialExporter.h"


/**
  Specialized exporter of the Metal material for Luxcore renderer
 */
namespace Reality {

class REALITY_LIB_EXPORT ReMetalLuxcoreExporter : public ReLuxMaterialExporter {

public:
  QString getMetalColor( ReMetal::MetalType metalType ) {
    QString metalColor;
    switch(metalType) {
      case ReMetal::Aluminum:
        metalColor = "0.65 0.65 0.65";
        break;
      case ReMetal::Chrome:
        metalColor = "0.45 0.45 0.45";
        break;
      case ReMetal::Cobalt:
        metalColor = "0.57 0.56 0.54";
        break;
      case ReMetal::Copper:
        metalColor = "0.62 0.51 0.49";
        break;
      case ReMetal::Gold:
        metalColor = "0.92 0.80 0.56";
        break;
      case ReMetal::Lead:
        metalColor = "0.55 0.57 0.61";
        break;
      case ReMetal::Mercury:
        metalColor = "0.45 0.47 0.45";
        break;
      case ReMetal::Nickel:
        metalColor = "0.61 0.58 0.56";
        break;
      case ReMetal::Platinum:
        metalColor = "0.48 0.49 0.46";
        break;
      case ReMetal::Silver:
        metalColor = "0.50 0.50 0.49";
        break;
      case ReMetal::Titanium:
        metalColor = "0.47 0.45 0.42";
        break;
      case ReMetal::Zinc:
        metalColor = "0.49 0.52 0.55";
        break;
      case ReMetal::Custom:
        break;
    }
    return metalColor;
  }

  void exportBaseMaterial( QString& str, const ReMetal* mat, const QString matName )  {

    ReMetal::MetalType mType = mat->getMetalType();
    QString fresnelTextureName = QString("%1_fresnel").arg(matName);

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
    fresnelTextureName.replace('\\', '/');
#endif

    if (mType == ReMetal::Custom) {
      str += QString(
               "Texture \"%1\" \"fresnel\" \"fresnelcolor\" \"texture Kr\" [\"%2\"]\n"
             )
             .arg(fresnelTextureName)
             .arg(ReLuxTextureExporter::getTextureFromCache(
                    mat->getKr()
                  )->getUniqueName());
    }
    else {
      QString metalColor = getMetalColor(mType);
      str += QString(
               "Texture \"%1\" \"fresnel\" \"fresnelcolor\" \"color Kr\" [%2]\n"
             )
             .arg(fresnelTextureName)
             .arg(metalColor);
    }
    str += QString("# Luxcore Mat: %1 (Metal2)\n").arg(matName);

    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"metal2\"] "
             "\"texture fresnel\" [\"%2\"] "
           )
           .arg(matName)
           .arg(fresnelTextureName);             

    // The polish is expressed in the UI with a range of 0-10000. Lux uses a "roughness"
    // parameter, opposite in idea, which is in the range of 0.0-1.0. The following
    // formulas convert from one system to the other
    double hr = 1.0 - mat->getHPolish()/10000.0;
    double vr = 1.0 - mat->getVPolish()/10000.0;
    // If the roughness approaches 0 then Lux will render the surface as black. We apply a small 
    // correction for that case
    if (hr < 0.0005) {
      hr = 0.0008;
    };
    if (vr < 0.0005) {
      vr = 0.0008;
    };
    str += QString(" \"float uroughness\" [%1] \"float vroughness\" [%2]\n")
             .arg(hr)
             .arg(vr);  

  }

  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    result = QString();
    if (basemat->getType() != MatMetal) {
      return;
    }
    auto mat = static_cast<const ReMetal*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();

    // Alpha channel/map handling...
    ReTexturePtr alphaMap = ReLuxTextureExporter::getTextureFromCache(
                              mat->getAlphaMap()
                            );
    float alphaStrength = mat->getAlphaStrength();

    if (!alphaMap.isNull() || alphaStrength < 1.0) {
      exportBaseMaterial(str, mat, QString("%1_BaseMat").arg(matName));
      str += makeAlphaChannel(matName, alphaMap, alphaStrength);
      result = str;
      return;
    }

    exportBaseMaterial(str, mat, matName);

    // Bump map.The method needs to write several textures into the scene 
    // before the material is exported.
    ReTexturePtr bumpMap = ReLuxTextureExporter::getTextureFromCache(
                             mat->getBumpMap()
                           );
    QString bmTextures;
    if (!bumpMap.isNull()) {
      QString bumpMapTexName = makeBumpMap(mat,
                                           bumpMap, 
                                           bmTextures, 
                                           mat->getBmNegative(), 
                                           mat->getBmPositive(), 
                                           mat->getBmStrength());
      str += QString(" \"texture bumpmap\" [\"%1\"] ").arg(bumpMapTexName);
    }

    // Displacement map. The method needs to write several textures into the scene 
    // before the material is exported.
    QString dmTextures;
    if (RealitySceneData->isDisplacementEnabled()) {
      ReTexturePtr dispMap = ReLuxTextureExporter::getTextureFromCache(
                               mat->getDisplacementMap()
                             );
      if (!dispMap.isNull()) {
         makeDispMap( mat,
                      dispMap, 
                      dmTextures, 
                      mat->getDmNegative(), 
                      mat->getDmPositive(), 
                      mat->getDmStrength() );
      }
    }

    result = QString(bmTextures % dmTextures % str);
  }
};

}

#endif
