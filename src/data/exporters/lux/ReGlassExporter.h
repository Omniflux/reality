/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_GLASS_LUX_EXPORTER_H
#define RE_GLASS_LUX_EXPORTER_H

#include <boost/any.hpp>

#include "reality_lib_export.h"
#include "ReGlass.h"
#include "ReMaterial.h"
#include "ReTexture.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "exporters/lux/ReLuxTextureExporter.h"


namespace Reality {

/**
 * Exports a Glass material to LuxRender
 */
class REALITY_LIB_EXPORT ReGlassLuxExporter : public ReLuxMaterialExporter {
private:
  const QString exportBaseMaterial( const ReGlass* mat, const QString matName ) {
    QString str;

    str = QString("# Mat: %1\n").arg(matName);

    QString glassType;
    bool isArchi = false;
    auto kr = ReLuxTextureExporter::getTextureFromCache(mat->getKr());
    auto kt = ReLuxTextureExporter::getTextureFromCache(mat->getKt());

    switch(mat->getGlassType()) {
      case ReGlass::ArchitecturalGlass:
        isArchi = true;
        // lack of break is intentional
      case ReGlass::StandardGlass: {
        str = QString(
                "MakeNamedMaterial \"%1\" \"string type\" [\"glass\"] \"float index\" [%2] "
                "\"texture Kr\" [\"%3\"] \"texture Kt\" [\"%4\"] "
              )
              .arg(matName)
              .arg(mat->getIOR())
              .arg(kr->getUniqueName())
              .arg(kt->getUniqueName());
        if (isArchi) {
          str += "\"bool architectural\" [\"true\"] ";
        }
        else if (mat->hasDispersion()) {
          str += QString("\"float cauchyb\" [%1] ")
                 .arg(mat->getCauchyB());
        }
        auto filmIOR = mat->getThinFilmIOR();
        auto filmThickness = mat->getThinFilmThickness();
        if (filmIOR > 0.0 && filmThickness > 0.0) {
          str += QString("\"float film\" [%1] \"float filmindex\" [%2] ")
                   .arg(filmThickness,0,'f',3)
                   .arg(filmIOR,0,'f',3);
        }
        break;
      }
      case ReGlass::FrostedGlass: {
        str = QString(
                "MakeNamedMaterial \"%1\" \"string type\" [\"roughglass\"] "
                "\"float uroughness\" [%2] \"float vroughness\" [%3] \"float index\" [%4] "
                "\"texture Kr\" [\"%5\"] \"texture Kt\" [\"%6\"] "
              )
              .arg(matName)
              .arg(mat->getURoughness())
              .arg(mat->getVRoughness())
              .arg(mat->getIOR())
              .arg(kr->getUniqueName())
              .arg(kt->getUniqueName());
        if (mat->hasDispersion()) {
          str += QString("\"bool dispersion\" [\"true\"] \"float cauchyb\" [%1] ")
                 .arg(mat->getCauchyB());
        }
        break;
      }
      case ReGlass::VolumetricGlass:
        str += QString(
                "MakeNamedMaterial \"%1\" \"string type\" [\"glass2\"] "
              )
              .arg(matName);
        if (mat->hasDispersion()) {
          str += QString("\"bool dispersion\" [\"true\"] ");
        }
        break;
    }
    // Bump map.The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    ReTexturePtr bumpMap = mat->getBumpMap();
    QString bmTextures;
    if (!bumpMap.isNull()) {
      QString bumpMapTexName = makeBumpMap(
                                 mat,
                                 bumpMap, 
                                 bmTextures, 
                                 mat->getBmNegative(), 
                                 mat->getBmPositive(), 
                                 mat->getBmStrength()
                               );
      str += QString(" \"texture bumpmap\" [\"%1\"] ").arg(bumpMapTexName);
    }
  
    // Displacement map. The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    QString dmTextures;
    if (RealitySceneData->isDisplacementEnabled()) {
      ReTexturePtr dispMap = mat->getDisplacementMap();
      if (!dispMap.isNull()) {
         makeDispMap( mat,
                      dispMap, 
                      dmTextures, 
                      mat->getDmNegative(), 
                      mat->getDmPositive(), 
                      mat->getDmStrength() );
      }
    }

    str += "\n";
    return QString(bmTextures % dmTextures % str);
  }

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatGlass) {
      result = QString();
      return;
    }
    auto mat = static_cast<const ReGlass*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();

    // Alpha channel/map handling...
    ReTexturePtr alphaMap = ReLuxTextureExporter::getTextureFromCache(
                              mat->getAlphaMap()
                            );
    float alphaStrength = mat->getAlphaStrength();

    if (!alphaMap.isNull() || alphaStrength < 1.0) {
      str = exportBaseMaterial(mat, QString("%1_BaseMat").arg(matName));
      str += makeAlphaChannel(matName, alphaMap, alphaStrength);
    }
    else {
      // No alpha channel, export the base material as it is
      str = exportBaseMaterial(mat, matName);
    }
    result = str;
  }
};

}

#endif
