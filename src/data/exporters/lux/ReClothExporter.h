#ifndef RE_CLOTH_LUX_EXPORTER_H
#define RE_CLOTH_LUX_EXPORTER_H

#include "ReCloth.h"
#include "exporters/lux/ReLuxMaterialExporter.h"

/**
  Exports a material of type ReCloth to LuxRender.
 */
namespace Reality {

class RE_LIB_ACCESS ReClothLuxExporter : public ReLuxMaterialExporter {
private:
  const ReCloth* mat;
public:
  
  const QString exportBaseMaterial( const QString matName )  {
    QString str = QString("# Mat: %1\n").arg(matName);

    auto warpKd = ReLuxTextureExporter::getTextureFromCache(mat->getWarp_Kd());
    auto warpKs = ReLuxTextureExporter::getTextureFromCache(mat->getWarp_Ks());
    auto weftKd = ReLuxTextureExporter::getTextureFromCache(mat->getWeft_Kd());
    auto weftKs = ReLuxTextureExporter::getTextureFromCache(mat->getWeft_Ks());
    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"cloth\"] "
             "  \"string presetname\" [\"%2\"]\n"
             "  \"texture warp_Kd\" [\"%3\"]\n"
             "  \"texture warp_Ks\" [\"%4\"]\n"
             "  \"texture weft_Kd\" [\"%5\"]\n"
             "  \"texture weft_Ks\" [\"%6\"]\n"
             "  \"float repeat_u\"  [%7]\n" 
             "  \"float repeat_v\"  [%8]\n" 
           )
           .arg(matName)
           .arg(mat->getPresetName())
           .arg(warpKd->getUniqueName()) 
           .arg(warpKs->getUniqueName()) 
           .arg(weftKd->getUniqueName()) 
           .arg(weftKs->getUniqueName())
           .arg(mat->getURepeat())
           .arg(mat->getVRepeat());
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
    // before the material is exported.
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

  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatCloth) {
      result = QString();
      return;
    }
    mat = static_cast<const ReCloth*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();

    // Alpha channel/map handling...
    ReTexturePtr alphaMap = ReLuxTextureExporter::getTextureFromCache(
                              mat->getAlphaMap()
                            );
    float alphaStrength = mat->getAlphaStrength();

    if (!alphaMap.isNull() || alphaStrength < 1.0) {
      str = exportBaseMaterial(QString("%1_BaseMat").arg(matName));
      str += makeAlphaChannel(matName, alphaMap, alphaStrength);
    }
    else {
      // No alpha channel, export the base material as it is
      str = exportBaseMaterial(matName);
    }
    result = str;
  }
};

}

#endif
