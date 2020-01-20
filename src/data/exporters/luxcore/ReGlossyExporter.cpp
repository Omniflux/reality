/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReGlossyExporter.h"

namespace Reality {

void ReGlossyLuxcoreExporter::exportKs( QString& ksTextures, QString& ksValue ) {
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
  // For the Luxcore version of this material we ignore the texture used to regulate  
  // the glossiness. 
  uGloss = (10000.0f - ugl)/10000.0f;
  vGloss = (10000.0f - vgl)/10000.0f;

  ksTextures = "";
  ksValue = QString(
             "  \"texture Ks\" [\"%1\"]\n"
             "  \"float index\" [0.0]\n"
             "  \"float uroughness\" [%2]\n"
             "  \"float vroughness\" [%3]\n"
            )
            .arg(ReLuxTextureExporter::getTextureFromCache(
                  mat->getKs()
                 )->getUniqueName())     
            .arg(uGloss)
            .arg(vGloss);
  
}

const QString ReGlossyLuxcoreExporter::exportBaseMaterial( const QString matName )  {
  QString str = QString("# Luxcore Mat: %1 (Glossy)\n").arg(matName);

  str += QString("MakeNamedMaterial \"%1\" \"string type\" [\"glossy\"] \"bool separable\" [\"true\"] ")
           .arg(matName);      
  // Diffuse map
  str += QString("\"texture Kd\" [\"%1\"] ")
           .arg(ReLuxTextureExporter::getTextureFromCache(
                  mat->getKd()
                )->getUniqueName());

  // Specular map
  QString ksTextures, ksValue;  
  exportKs(ksTextures, ksValue);
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

  return ksTextures % bmTextures % dmTextures % str % ksValue;
}


void ReGlossyLuxcoreExporter::exportMaterial( const ReMaterial* basemat,
                                              boost::any& result )  
{
  result = QString();
  ReMaterialType matType = basemat->getType();
  if ( matType != MatGlossy && matType != MatSkin ) {
    return;
  }  
  mat = static_cast<const Glossy*>(basemat);
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

}; // namespace
