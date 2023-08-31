/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReSkinExporter.h"

#include "ReLuxTextureExporterfactory.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "textures/ReImageMap.h"
#include "textures/ReMath.h"
#include "textures/ReMix.h"


using namespace Reality;

void ReSkinLuxExporter::makeHairMaskTexture( const QString& texName, 
                                             const QString& matName,
                                             const ReSkin* mat,
                                             QString& result ) 
{
  // The bw version of the source texture
  QString floatHairMaskName = QString("%1_floatHairmask").arg(matName);
  ImageMapPtr floatHairMask = ImageMapPtr( new ImageMap(floatHairMaskName) );
  floatHairMask->setFileName(mat->getKhmImageMap());
  floatHairMask->setTextureDataType(ReTexture::numeric);
  floatHairMask->setGain(mat->getHmGain());

  // Square of the bw texture to create a high-contrast version
  QString squaredTex = QString("%1_squared").arg(floatHairMaskName);
  ReMathPtr bwSquared = ReMathPtr( new ReMath(squaredTex) );
  bwSquared->setTexture1(floatHairMask);
  bwSquared->setAmount1(1.4);
  bwSquared->setTexture2(floatHairMask);
  bwSquared->setAmount2(1.4);
  bwSquared->setFunction(ReMath::multiply);

  // Second Square of the bw texture to create a high-contrast version
  QString squaredTex2 = QString("%1_squared2").arg(floatHairMaskName);
  ReMathPtr bwSquared2 = ReMathPtr( new ReMath(squaredTex2) );
  bwSquared2->setTexture1(bwSquared);
  bwSquared2->setAmount1(1.4);
  bwSquared2->setTexture2(bwSquared);
  bwSquared2->setAmount2(1.4);
  bwSquared2->setFunction(ReMath::multiply);

  // third Square of the bw texture to create a high-contrast version
  QString squaredTex3 = QString("%1_squared3").arg(floatHairMaskName);
  ReMathPtr bwSquared3 = ReMathPtr( new ReMath(squaredTex3) );
  bwSquared3->setTexture1(bwSquared2);
  bwSquared3->setAmount1(1.4);
  bwSquared3->setTexture2(bwSquared2);
  bwSquared3->setAmount2(1.4);
  bwSquared3->setFunction(ReMath::multiply);

  // Convert the hairMask to a color texture via a mix texture. A Mix
  // texture uses black and white automatically for the tex1 and tex2
  // channels so we don't need to set them explicitly.
  ReMixTexturePtr hairMask = ReMixTexturePtr( 
                               new ReMixTexture(texName,0, ReTexture::color) 
                             );
  hairMask->setMixTexture(bwSquared3);

  auto imTexExporter = ReLuxTextureExporterFactory::getExporter(floatHairMask);
  auto sqTexExporter = ReLuxTextureExporterFactory::getExporter(bwSquared);
  auto mixTexExporter = ReLuxTextureExporterFactory::getExporter(hairMask);

  result += "# Hair mask\n";
  result += imTexExporter->exportTexture(floatHairMask, floatHairMaskName) + "\n";
  result += sqTexExporter->exportTexture(bwSquared, squaredTex) + "\n";
  result += sqTexExporter->exportTexture(bwSquared2, squaredTex2) + "\n";
  result += sqTexExporter->exportTexture(bwSquared3, squaredTex3) + "\n";
  result += mixTexExporter->exportTexture(hairMask, texName) + "\n";
}

const QString ReSkinLuxExporter::addMaterial1( const QString& matName, 
                                               const QString& bmTexName ) 
{
  QString specialTextures;

  QString str = QString(
                         "MakeNamedMaterial \"%1\" \"string type\" [\"glossytranslucent\"]\n"
                         "  \"bool onesided\" [\"true\"]\n"
                         "  \"bool multibounce\" [\"false\"]\n"
                         "  \"texture Kd\" [\"%2\"]\n"
                         "  \"float d\" [0.25]\n"
                         // "  \"color Ka\" [0.01444 0.02843 0.2232]\n"
                       )
                       .arg( matName )
                       .arg( ReLuxTextureExporter::getTextureFromCache(
                               mat->getKd()
                             )->getUniqueName() );

  QPair<QString, QString> kaValues = exportKa();
  str += kaValues.first;
  specialTextures += kaValues.second;

  // Automatically generate the hair mask texture                       
  if (mat->hasHairMask()) {
    QString hairMaskName = QString("%1_hairmask").arg(matName);
    makeHairMaskTexture(hairMaskName, matName, mat, specialTextures);
    str += QString("  \"texture Kt\" [\"%1\"]\n").arg(hairMaskName);
  }
  else {
    str += QString("  \"color Kt\" [1.0 1.0 1.0]\n");
  }
  QString ksTextures, ksValue;
  // Specular map
  exportKs( ksTextures, ksValue );
  str += ksValue;
  if ( bmTexName != "" ) {
    str += QString("  \"texture bumpmap\" [\"%1\"]\n").arg(bmTexName);    
  }
  return ksTextures % specialTextures % str;
}

const QString ReSkinLuxExporter::addMaterial2( const QString& matName, 
                                               const QString& bmTexName ) {
  QString kdName = ReLuxTextureExporter::getTextureFromCache(
                     mat->getKd() 
                   )
                   ->getUniqueName();
  QString str = QString(
                  "Texture \"%1_fresnel\" \"fresnel\" \"fresnelcolor\" "
                  "\"texture Kr\" [\"%2\"]\n"

                  "MakeNamedMaterial \"%1\" \"string type\" [\"metal2\"] "
                  "  \"float uroughness\" [0.25] \"float vroughness\" [0.25] "
                  "  \"texture fresnel\" [\"%1_fresnel\"]\n"
                )
                .arg( matName )
                .arg( kdName );
  if ( bmTexName != "" ) {
    str += QString("  \"texture bumpmap\" [\"%1\"]\n").arg(bmTexName);    
  }
  return str;
}

const QString ReSkinLuxExporter::exportWithSSS( const QString matName ) {
  QString str = QString("# Mat: %1 (Skin w/SSS)\n").arg(matName);

  // Bump map.The method needs to write several textures into the scene 
  // before the material is exported. This must NOT fetch the texture
  // from the cache
  ReTexturePtr bumpMap = mat->getBumpMap();
  
  QString bmTextures;
  QString bmTexName;
  if (!bumpMap.isNull()) {
    bmTexName = makeBumpMap( mat,
                             bumpMap, 
                             bmTextures, 
                             mat->getBmNegative(), 
                             mat->getBmPositive(), 
                             mat->getBmStrength() );
  }

  // Displacement map. The method needs to write several textures into the scene 
  // before the material is exported. This must NOT fetch the texture
  // from the cache
  ReTexturePtr dispMap = mat->getDisplacementMap();

  QString dmTextures;
  if (!dispMap.isNull()) {
    makeDispMap( mat,
                 dispMap, 
                 dmTextures, 
                 mat->getDmNegative(), 
                 mat->getDmPositive(), 
                 mat->getDmStrength() );
  }

  QString mat1Name = QString("%1_dermis").arg(matName);
  QString mat2Name = QString("%1_floatfresnel").arg(matName);
  // Material 1: Glossy Translucent
  str += addMaterial1( mat1Name, bmTexName );
  
  // Material 2: Metal2
  str += addMaterial2( mat2Name, bmTexName );
  str += QString(
           "MakeNamedMaterial \"%1\" \"string type\" [\"mix\"]\n"
           "  \"string namedmaterial1\" [\"%2\"]\n"
           "  \"string namedmaterial2\" [\"%3\"]\n"
           "  \"float amount\" [%4]\n"
         )
         .arg(matName)
         .arg(mat1Name)
         .arg(mat2Name)
         .arg(mat->getFresnelAmount());

  return bmTextures % dmTextures % str;
}

const QString ReSkinLuxExporter::exportBaseMaterial( const QString matName ) {

  // SSS support
  if (mat->isSSSEnabled()) {
    return exportWithSSS( matName );
  }

  QString ksTextures, ksValue;

  QString str = QString("# Mat: %1 (Skin)\n").arg(matName);

  str += QString(
           "MakeNamedMaterial \"%1\" \"string type\" [\"glossy\"] \"bool separable\" [\"true\"] "
         ).arg(matName);

  QPair<QString, QString> kaValues = exportKa();
  str += kaValues.first;
  QString specialTextures = kaValues.second;

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
  exportKs(ksTextures, ksValue);

  // Bump map.The method needs to write several textures into the scene 
  // before the material is exported. This must NOT fetch the texture
  // from the cache
  ReTexturePtr bumpMap = ReLuxTextureExporter::getTextureFromCache(
                           mat->getBumpMap()
                         );
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
  // before the material is exported. 
  QString dmTextures;
  if (RealitySceneData->isDisplacementEnabled()) {
    ReTexturePtr dispMap = ReLuxTextureExporter::getTextureFromCache(
                             mat->getDisplacementMap()
                           );
    if (!dispMap.isNull()) {
      makeDispMap( mat,
                   dispMap, 
                   dmTextures, 
                   mat->getDmNegative(), 
                   mat->getDmPositive(), 
                   mat->getDmStrength() );
    }
  }
  return ksTextures % bmTextures % dmTextures % specialTextures % str % ksValue;
}

void ReSkinLuxExporter::exportMaterial( const ReMaterial* basemat, 
                                        boost::any& result ) 
{
  if (basemat->getType() != MatSkin) {
    result = QString();
    return;
  }
  mat = static_cast<const ReSkin*>(basemat);
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
    str = exportBaseMaterial(matName);
  }
  result = str;
}
