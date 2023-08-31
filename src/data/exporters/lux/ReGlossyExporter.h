/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */


#ifndef RE_GLOSSY_LUX_EXPORTER_H
#define RE_GLOSSY_LUX_EXPORTER_H

#include <QPair>

#include "reality_lib_export.h"
#include "ReGlossy.h"
#include "ReTexture.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReColorMath.h"
#include "textures/ReConstant.h"

namespace boost {
  class any;
}

namespace Reality {
  class ReMaterial;
}


#define RE_GLOSSINESS_MAX_LIMIT 10000.0f

namespace Reality {

/**
  Class: ReGlossyLuxExporter
 */
template <typename T> 
class REALITY_LIB_EXPORT ReBasicGlossyLuxExporter : public ReLuxMaterialExporter {

protected:
  const T* mat;

  QPair<QString, QString> exportKa() {
    QPair<QString, QString> retval;
    // Coat
    auto matKa = mat->getKa();
    if (mat->hasTopCoat() && !matKa.isNull()) {
      ReTexturePtr Ka = ReLuxTextureExporter::getTextureFromCache(matKa);

      // QColor KaColor;
      // double r,g,b;
      float coatDepth = mat->getCoatThickness();

      // Invert the RGB values because Lux uses an absorption concept and we 
      // use the color of the coat
      ReColorMathPtr invTex = ReColorMathPtr(
                                new ReColorMath(
                                      QString("%1:%2_invert")
                                        .arg(mat->getUniqueName())
                                        .arg(Ka->getName()),
                                      NULL
                                    )
                              );
      invTex->setFunction(ReColorMath::subtract);
      auto whiteColor = ReTexturePtr(
                          new ReConstant(
                                QString("%1_inv_tex1").arg(mat->getUniqueName()),
                                0,
                                RE_PURE_WHITE_COLOR
                              )
                        );
      invTex->setTexture1(whiteColor);
      invTex->setColor1(RE_PURE_WHITE_COLOR);
      invTex->setTexture2(Ka);
      invTex->setColor2(RE_PURE_WHITE_COLOR);

      auto invTexExporter = ReLuxTextureExporterFactory::getExporter(invTex);
      auto whiteExporter = ReLuxTextureExporterFactory::getExporter(whiteColor);
      retval.second += whiteExporter->exportTexture(whiteColor) + "\n";
      retval.second += invTexExporter->exportTexture(invTex) + "\n";

      retval.first += QString(
               "  \"texture Ka\" [\"%1\"] \"float d\" [%2]\n"
             )
             .arg(invTex->getUniqueName())
             .arg(coatDepth);
    }
    return retval;
  }

public:
  
  void exportKs( QString& ksTextures, QString& ksValue ) {
    int ugl = mat->getUGlossiness();
    int vgl = mat->getVGlossiness();
    double uGloss;
    double vGloss;

    ReTexturePtr glossinessTex = ReLuxTextureExporter::getTextureFromCache(
                                   mat->getKg()
                                 );
    // We calculate the glossiness as a roughness value. This is because Lux
    // actually express this value with the counter-intuitive "roughness" concept.
    // The roughness value is expressed as a float from 0.0, no roughness, to 1.0,
    // fully rough material.
    // Since we make things easier for the user by expressing the value as an integer 
    // from 0, no glossiness, aka rough material, to 10000, mirror-finish glossiness,
    // we convert between the two systems with the calculation below. 
    // 
    // This task is made more complex by the possibility of the presence of a texture
    // used to regulate the glossiness. If such texture, named Kg, exists, then the
    // texture is interpreted as follows:
    //   - A black pixel in the texture indicates no glossiness, so the surface at 
    //     that point will be rough
    //   - A pure white pixel indicates that the surface is glossy to the values
    //     expressed in the uGlossiness:vGlossiness pair.
    // For this to work, though, we need to flip the texture because 0, the value
    // for black, is actually 0 roughness in Lux. The solution is to use the 
    // Mix texture with 1.0 as the value for input 0 and the maximum roughness 
    // for input value 1.0. This results in a texture that is inverted and that
    // goes from no shine to the maximum amount of shine selected and with
    // the texture driving the values.

    // Workaround to avoid rounding errors on Lux. 
    if (ugl == RE_GLOSSINESS_MAX_LIMIT) {
      ugl--;
    }
    if (vgl == RE_GLOSSINESS_MAX_LIMIT) {
      vgl--;
    }
    uGloss = (RE_GLOSSINESS_MAX_LIMIT - ugl)/RE_GLOSSINESS_MAX_LIMIT;
    vGloss = (RE_GLOSSINESS_MAX_LIMIT - vgl)/RE_GLOSSINESS_MAX_LIMIT;

    bool anisotropic = uGloss != vGloss;
    QString ksName = ReLuxTextureExporter::getTextureFromCache(
                       mat->getKs() 
                     )
                     ->getUniqueName();

    // Most common case first, the user selected just numeric values
    // for the glossiness
    if (glossinessTex.isNull()) {
      ksTextures = "";
      ksValue = QString(
                 "  \"texture Ks\" [\"%1\"]\n"
                 // "  \"float index\" [0.0]\n"
                 "  \"float uroughness\" [%2]\n"
                 "  \"float vroughness\" [%3]\n"
                )
                .arg(ksName)
                .arg(uGloss)
                .arg(vGloss);
      return;
    }
    else {
      // We have values and a texture to drive them along the material.
      QString uGlossTex, vGlossTex;

      // uGlossiness texture
      uGlossTex = QString("%1_uGloss").arg(mat->getUniqueName());
      vGlossTex = uGlossTex;
      
      // To avoid conflict with other textures using this texture we need to 
      // duplicate it instead of sharing it. In this way the same texture can 
      // be used as a specular texture (type color) and to drive the glossiness
      // value (type float)
      QString glossinessTextureName = glossinessTex->getUniqueName(); 
      if ( glossinessTex->getDataType() != ReTexture::numeric ) {
        glossinessTextureName = ReLuxTextureExporter::writeFloatVersion(                                
                                  glossinessTex, 
                                  ksTextures
                                )->getUniqueName();
      }
      // Create the inverted texture...
      ksTextures += QString(
                      "Texture \"%1\" \"float\" \"mix\" \"float tex1\" [1.0] "
                      "\"float tex2\" [%2] \"texture amount\" [\"%3\"]\n"
                    )
                    .arg(uGlossTex)
                    .arg(uGloss)
                    .arg(glossinessTextureName);

      // vGlossiness texture, if the glossiness is anisotropic
      if (anisotropic) {
        vGlossTex = QString("%1_vGloss").arg(mat->getUniqueName());
        ksTextures += QString(
                      "Texture \"%1\" \"float\" \"mix\" \"float tex1\" [1.0] "
                      "\"float tex2\" [%2] \"texture amount\" [\"%3\"]\n"
                    )
                    .arg(vGlossTex)
                    .arg(vGloss)
                    .arg(glossinessTextureName);

      }
      ksValue += QString(
                   "  \"texture Ks\" [\"%1\"]\n"
                   // "  \"float index\" [0.0]\n"
                   "  \"texture uroughness\" [\"%2\"]\n"
                   "  \"texture vroughness\" [\"%3\"]\n"
                 )
                 .arg(ksName)
                 .arg(uGlossTex)
                 .arg(vGlossTex);
    }
  }

};


class ReGlossyLuxExporter : public ReBasicGlossyLuxExporter<Glossy> {

protected:
  const QString exportBaseMaterial( const QString matName );

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result );

};


}

#endif
