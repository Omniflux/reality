/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReGlossyExporter.h"

#include <boost/any.hpp>

#include "ReMaterial.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"


namespace Reality {

const QString ReGlossyLuxExporter::exportBaseMaterial( const QString matName )  {
  QString str = QString("# Mat: %1 (Glossy)\n").arg(matName);

  if (mat->isTranslucent() && !mat->getKt().isNull()) {
    str += QString("MakeNamedMaterial \"%1\" \"string type\" [\"glossytranslucent\"] \"bool onesided\""
                   "[\"false\"] ")
                   .arg(matName);
    ReTexturePtr Kt = ReLuxTextureExporter::getTextureFromCache(mat->getKt());
    str += QString("\"texture Kt\" [\"%1\"] ").arg(Kt->getUniqueName());
  }
  else {
    // str += QString("MakeNamedMaterial \"%1\" \"string type\" [\"glossy\"] \"bool separable\" [\"true\"] ")
    //          .arg(matName);
    str += QString("MakeNamedMaterial \"%1\" \"string type\" [\"glossy\"] ")
             .arg(matName);
  }

  // Multibounce support
  if (mat->getSurfaceFuzz()) {
    str += "\"bool multibounce\" [\"true\"] ";
  }
  // Diffuse map
  QString kdName = ReLuxTextureExporter::getTextureFromCache(
                     mat->getKd() 
                   )
                   ->getUniqueName();
  str += QString("\"texture Kd\" [\"%1\"] ").arg(kdName);

  // Specular map
  QString ksTextures, ksValue;  
  exportKs(ksTextures, ksValue);

  // Coat
  QPair<QString, QString> kaValues = exportKa();
  str += kaValues.first;
  ksTextures += kaValues.second;

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


void ReGlossyLuxExporter::exportMaterial( const ReMaterial* basemat, boost::any& result )  {
  if (basemat->getType() != MatGlossy) {
    result = QString();
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
