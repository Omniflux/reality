/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MATTE_LUX_EXPORTER_H
#define RE_MATTE_LUX_EXPORTER_H

//#include "ReMaterials.h"
#include "ReMatte.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "reality_lib_export.h"

/**
  Exports the Matte material to Lux
 */
namespace Reality {

class REALITY_LIB_EXPORT ReMatteLuxExporter : public ReLuxMaterialExporter {
private:
  const QString exportBaseMaterial( const ReMatte* mat, const QString matName )  {
    QString str = QString("# Mat: %1\n").arg(matName);
    auto kd = ReLuxTextureExporter::getTextureFromCache(mat->getKd());
    auto kt = ReLuxTextureExporter::getTextureFromCache(mat->getKt());
    if (mat->isTranslucent()) {
      str += QString(
              "MakeNamedMaterial \"%1\" \"string type\" [\"mattetranslucent\"]\n"
              "  \"texture Kr\" [\"%3\"] \"float sigma\" [%4] \n"
              "  \"texture Kt\" [\"%5\"] \"bool energyconserving\" [\"%6\"]\n"              
            )
            .arg(matName)
            .arg(kd->getUniqueName())
            .arg(mat->getRoughness())
            .arg(kt->getUniqueName())
            .arg(mat->conservesEnergy() ? "true" : "false");
    }
    else {
      str += QString(
               "MakeNamedMaterial \"%1\" \"string type\" [\"matte\"] "
               "\"texture Kd\" [\"%2\"] \"float sigma\" [%3] \n"
             )
             .arg(matName)
             .arg(kd->getUniqueName())
             .arg(mat->getRoughness());      
    }
    // Bump map.The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    ReTexturePtr bumpMap = mat->getBumpMap();
    QString bmTextures;
    if (!bumpMap.isNull()) {
      QString bumpMapTexName = makeBumpMap( mat,
                                            bumpMap, 
                                            bmTextures, 
                                            mat->getBmNegative(), 
                                            mat->getBmPositive(), 
                                            mat->getBmStrength() );
      str += QString(" \"texture bumpmap\" [\"%1\"] ").arg(bumpMapTexName);
    }

    // Displacement map. The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    ReTexturePtr dispMap = mat->getDisplacementMap();
    QString dmTextures;
    if (!dispMap.isNull()) {
      QString dispMapTexName = makeDispMap( mat,
                                            dispMap, 
                                            dmTextures, 
                                            mat->getDmNegative(), 
                                            mat->getDmPositive(), 
                                            mat->getDmStrength() );
    }

    return bmTextures % dmTextures % str;
  }
public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatMatte) {
      result = QString();
      return;
    }
    auto mat = static_cast<const ReMatte*>(basemat);
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
