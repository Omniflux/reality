  #ifndef RE_GLASS_LUXCORE_EXPORTER_H
#define RE_GLASS_LUXCORE_EXPORTER_H

#include "ReGlass.h"
#include "exporters/luxcore/ReLuxcoreMaterialExporter.h"

/**
  Exporter for the Luxcore version of the Glass material.
 */

namespace Reality {

class RE_LIB_ACCESS ReGlassLuxcoreExporter : public ReLuxcoreMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    result = QString();
    if (basemat->getType() != MatGlass) {
      return;
    }
    auto mat = static_cast<const ReGlass*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();
    str = QString("# Luxcore Mat: %1\n").arg(matName);

    QString glassType;
    bool isArchi = false;
    switch(mat->getGlassType()) {
      case ReGlass::ArchitecturalGlass:
        isArchi = true;
        // lack of break is intentional
      case ReGlass::VolumetricGlass:
      case ReGlass::StandardGlass:
        str = QString(
                "MakeNamedMaterial \"%1\" \"string type\" [\"glass\"] \"float index\" [%2] "
                "\"texture Kr\" [\"%3\"] \"texture Kt\" [\"%4\"] "
              )
              .arg(matName)
              .arg(mat->getIOR())
              .arg(ReLuxTextureExporter::getTextureFromCache(
                     mat->getKr()
                   )->getUniqueName())
              .arg(ReLuxTextureExporter::getTextureFromCache(
                     mat->getKt()
                   )->getUniqueName());
        if (isArchi) {
          str += "\"bool architectural\" [\"true\"] ";
        }
        else if (mat->hasDispersion()) {
          str += QString("\"float cauchyb\" [%1] ")
                 .arg(mat->getCauchyB());
        }
        break;
      case ReGlass::FrostedGlass:
        str = QString(
                "MakeNamedMaterial \"%1\" \"string type\" [\"roughglass\"] "
                "\"float uroughness\" [%2] \"float vroughness\" [%3] \"float index\" [%4] "
                "\"texture Kr\" [\"%5\"] \"texture Kt\" [\"%6\"] "
              )
              .arg(matName)
              .arg(mat->getURoughness())
              .arg(mat->getVRoughness())
              .arg(mat->getIOR())
              .arg(ReLuxTextureExporter::getTextureFromCache(
                     mat->getKr()
                   )->getUniqueName())
              .arg(ReLuxTextureExporter::getTextureFromCache(
                     mat->getKt()
                   )->getUniqueName());
        if (mat->hasDispersion()) {
          str += QString("\"bool dispersion\" [\"true\"] \"float cauchyb\" [%1] ")
                 .arg(mat->getCauchyB());
        }
        break;
    }
    // Bump map.The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    ReTexturePtr bumpMap = mat->getBumpMap();
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
    result = QString(bmTextures % dmTextures % str);
  }
};

}

#endif
