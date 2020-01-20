/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_CLOTH_LUXCORE_EXPORTER_H
#define RE_CLOTH_LUXCORE_EXPORTER_H

#include "ReCloth.h"
#include "exporters/luxcore/ReLuxcoreMaterialExporter.h"
#include "ReSceneResources.h"

/**
  Specialized exporter of the Cloth material for Luxcore renderer
  Cloth is not supported by LuxCore so we revert to Matte
 */
namespace Reality {

class RE_LIB_ACCESS ReClothLuxcoreExporter : public ReLuxMaterialExporter {

public:


  void exportBaseMaterial( QString& str, 
                           const ReCloth* mat, 
                           const QString matName )  
  {
    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"matte\"] "
             "\"texture Kd\" [\"%2\"] \"float sigma\" [%3] \n"
           )
           .arg(matName)
           .arg(ReLuxTextureExporter::getTextureFromCache(
                  mat->getWarp_Kd()
                )->getUniqueName())
           .arg(0.1);   
  }

  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    result = QString();
    if (basemat->getType() != MatCloth) {
      return;
    }
    const ReCloth* mat = static_cast<const ReCloth*>(basemat);
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

    result = QString(bmTextures % dmTextures % str);
  }
};

}

#endif
