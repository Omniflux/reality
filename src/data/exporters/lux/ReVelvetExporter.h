/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_VELVET_LUX_EXPORTER_H
#define RE_VELVET_LUX_EXPORTER_H

#include "reality_lib_export.h"
#include "ReLuxTextureExporter.h"
#include "ReMaterial.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReVelvet.h"
#include "exporters/lux/ReLuxMaterialExporter.h"


/*
  Class: ReVelvetLuxExporter
 */

namespace Reality {

class REALITY_LIB_EXPORT ReVelvetLuxExporter : public ReLuxMaterialExporter {
private:
  const ReVelvet* mat;

public:
  
  const QString exportBaseMaterial( const QString matName )  {
    QString str = QString("# Mat: %1\n").arg(matName);

    auto kd = ReLuxTextureExporter::getTextureFromCache(mat->getKd());
    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"velvet\"] "
             "\"float thickness\" [%2] "
             "\"texture Kd\" [\"%3\"] "
             "\"float p1\" [-2] \"float p2\" [10] \"float p3\" [2] "
           )
           .arg(matName)
           .arg(mat->getThickness())
           .arg(kd->getUniqueName()); 

    // Bump map.The method needs to write several textures into the scene 
    // before the material is exported.This must NOT fetch the texture
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

  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatVelvet) {
      result = QString();
      return;
    }
    mat = static_cast<const ReVelvet*>(basemat);
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
