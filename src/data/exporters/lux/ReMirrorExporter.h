/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MIRROR_LUX_EXPORTER_H
#define RE_MIRROR_LUX_EXPORTER_H

#include "ReMirror.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "reality_lib_export.h"

/*
  Class: ReMirrorLuxExporter
 */

namespace Reality {

class REALITY_LIB_EXPORT ReMirrorLuxExporter : public ReLuxMaterialExporter {

protected:
  const ReMirror* mat;
    
  const QString exportBaseMaterial( const QString matName ) {
    QString str;

    str = QString("# Mat: %1\n").arg(matName);
    auto kr = ReLuxTextureExporter::getTextureFromCache(mat->getKr());
    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"mirror\"] "
             "\"texture Kr\" [\"%2\"] \"float film\" [%3] "
             "\"float filmindex\" [%4] "
            )
            .arg(matName)
            .arg(kr->getUniqueName())
            .arg(mat->getFilmThickness())
            .arg(mat->getFilmIOR());

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
    // before the material is exported.This must NOT fetch the texture
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

    return bmTextures % dmTextures % str;
  }

public:

  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatMirror) {
      result = QString();
      return;
    }
    mat = static_cast<const ReMirror*>(basemat);
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
      str = QString(exportBaseMaterial(matName));
    }
    result = str;
  }
};

}

#endif
