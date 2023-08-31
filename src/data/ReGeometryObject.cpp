/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#include "ReGeometryObject.h"

#include <QJson/Parser>
#include <QJson/Serializer>

#include "ReAcsel.h"
#include "ReCloth.h"
#include "ReDisplaceableMaterial.h"
#include "ReGlass.h"
#include "ReGlossy.h"
#include "ReLightMaterial.h"
#include "ReMaterials.h"
#include "ReMaterialPropertyKeys.h"
#include "ReMatte.h"
#include "ReMetal.h"
#include "ReMirror.h"
#include "ReNodeConverter.h"
#include "ReNull.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReSkin.h"
#include "ReTextureCreator.h"
#include "ReTools.h"
#include "ReVelvet.h"
#include "ReWater.h"
#include "importers/qt/ReQtMaterialImporterFactory.h"
#include "importers/qt/ReVolumeImporter.h"
#include "textures/ReColorMath.h"
#include "textures/ReMix.h"


#define RE_SKIN_ADJUSTED_COAT_THICKNESS   0.20f
#define RE_GLOSSY_ADJUSTED_COAT_THICKNESS 0.50f
#define RE_SKIN_ADJUSTED_FRESNEL          0.07f

using namespace Reality;

QString ReGeometryObject::computeAcselID( const QString& matID, 
                                          const bool forDefaultShader ) 
{
  QStringList textures;
  // If we need an ID for the default shader then the textures are not
  // computed in the mix
  if (!forDefaultShader) {    
    getImageMaps(matInfo.diffuseMap, textures);
    // If an image map is not found then check if the texture is a constant
    // color. If it is then use the CSS-style name of the color to identify 
    // the texture
    if (!textures.count()) {
      if (matInfo.diffuseMap->getType() == TexConstant) {
        QColor x = matInfo.diffuseMap->getNamedValue("color").value<QColor>();
        textures << x.name();
      }
    }
    // Add the alpha map
    getImageMaps(matInfo.alphaMap, textures);
  }

  return ReAcsel::getInstance()->getAcselID(
    getGeometryFileName(), matID, textures
  );
}

void storeOriginalShader( const QString& objID, 
                          const QString& matName,
                          const QVariantMap& mat ) 
{
  QString matDef;
  QJson::Serializer toJson;
  ReAcsel::getInstance()->storeOriginalShader(
    objID, matName, toJson.serialize(mat)
  );
}

ReGeometryObject::ReGeometryObject( const QString name, 
                                    const QString internalName, 
                                    const QString geometryFile ) : 
  name(name), 
  internalName(internalName),
  geometryFile(geometryFile),
  visible(true)
{
  isLightFlag = name.startsWith(REALITY_LIGHT_PREFIX);
  lights = ReMaterialDictionaryPtr( new ReMaterialDictionary() );
  // Reserved for future use
  isHumanFigure = false;
};


ReGeometryObject::~ReGeometryObject() {
  removeOwnLights();
  // Unlink the linked volumes, if there are any
  ReMaterialIterator i(materials);
  while( i.hasNext() ) {
    i.next();
    ReMaterialPtr mat = i.value();
    if (mat.isNull()) {
      continue;
    }
    QString volName = mat->getInnerVolume();
    ReVolumePtr vol;
    if (volName != "") {
      RealitySceneData->unlinkVolumeFromMaterial(
        volName, internalName, mat->getName()
      );
    }
    volName = mat->getOuterVolume();
    if (volName != "") {
      RealitySceneData->unlinkVolumeFromMaterial(
        volName, internalName, mat->getName()
      );
    }
  }
}

void ReGeometryObject::rename( const QString newName ) {
  // Change the name, or label, of this object
  name = newName;

  // If the object is a mesh light then we need to rename the lights 
  // associated with it.
  if (lights->count()) {
    ReMaterialIterator li(*lights);
    while( li.hasNext() ) {
      li.next();
      ReLightMaterialPtr ml = li.value().staticCast<ReLightMaterial>();
      auto matLight = ml->getLight();
      matLight->setName( QString("%1_%2").arg(newName).arg(ml->getName()) );
    }    
  }

  // If the label starts with "RealityLight", then this object is a mesh light
  // and we need to convert the materials contained to emit lights.
  if ( name.startsWith(REALITY_LIGHT_PREFIX) ) {
    isLightFlag = true;
    ReMaterialIterator i(materials);
    while( i.hasNext() ) {
      i.next();
      auto matID = i.key();
      auto newMat = convertMaterial(matID, QVariantMap());
      // Delete the old material
      materials[matID].clear();
      // Store the new one
      materials[matID] = newMat;
      // If this is a material converted to light then we need to add it to the list
      // of lights as well
      lights->insert(matID, newMat);
      ReLightPtr matLight = newMat.staticCast<ReLightMaterial>()->getLight();
      // Add the light to the list of lights of the scene
      RealitySceneData->saveLight(matLight->getID(), matLight);
    }
  }
}

void ReGeometryObject::removeOwnLights() {
  // Remove the material lights...
  ReMaterialIterator li(*lights);  
  while( li.hasNext() ) {
    li.next();
    ReLightMaterialPtr ml = li.value().staticCast<ReLightMaterial>();
    RealitySceneData->deleteLight(ml->getLight()->getID(), false);
  }
  lights.clear();
}

void ReGeometryObject::unlinkLight( const QString& lightID ) {
  QString matID = "";
  ReMaterialIterator li(*lights);
  while( li.hasNext() ) {
    li.next();
    ReLightMaterialPtr ml = li.value().staticCast<ReLightMaterial>();
    if (ml->getLight()->getID() == lightID) {
      matID = li.key();
      break;
    }
  }
  if (matID != "") {
    lights->remove(matID);
  }
}

void ReGeometryObject::revertToOriginalShader( const QString matID ) {
  if (!materials.contains(matID)) {
    RE_LOG_WARN() << "Material does not exist for object " << name.toStdString();
    return;
  }
  auto acsel = ReAcsel::getInstance();
  QVariantMap shaderData;
  if (!acsel->getOriginalShader(getInternalName(), matID, shaderData)) {
    RE_LOG_WARN() << QString(
                       "Original shader definition for material %1 of %2"
                       " was not found"
                     )
                     .arg(matID)
                     .arg(getName())
                     .toStdString();
    return;
  }
  auto newMat = convertMaterial( matID, shaderData );
  auto oldMat = materials[matID];
  materials[matID] = newMat;
  oldMat.clear();
}
/*
//! This function combines two existing textures by creating a ReColorMath
//! texture.
//! \param texName The name of the combiner texture
//! \param t1 The first texture to combine
//! \param t2 The second texture to combine
//! \param function The type of operation to use for the mixing. It defaults
//!                 to multiplication.
ReColorMathPtr combineTextures( const QString& texName,
                                const ReTexturePtr t1, 
                                const ReTexturePtr t2,
                                ReColorMath::functions function = ReColorMath::multiply ) 
{
  ReColorMathPtr mixer = ReColorMathPtr( 
                           new Reality::ReColorMath(
                                 texName, 
                                 0,
                                 t1,
                                 t2,
                                 function
                               )
                         );
  return mixer;
}
*/

ReTexturePtr ReGeometryObject::convertSpecular( const QVariantMap& specData,
                                                const QString& matID,
                                                const QString& suffix ) 
{
  QString mapStr = specData["map"].toString();
  auto specularColor = convertFloatColor(specData["color"].toList());
  ReTexturePtr specularMap;

  if (mapStr != "") {
    ReTexturePtr specularTmp = ReNodeConverter::convertNode(
                                 mapStr, ReNodeConverter::ColorNode
                               );
    // Specular maps must have linear gamma
    if (specularTmp->getType() == TexImageMap) {
      specularTmp->setNamedValue("gamma", 1.0);
    }
    // If we have a specular color different than pure white then we need
    // to create a texture that is a mix of the color and the texture
    if (!isPureWhite(specularColor)) {
      // We use a ColorMath node because it has a built-in multiplication
      // of a color with another texture, similarly to what is in Poser
      specularMap = ReTexturePtr(new Reality::ReColorMath(
                                       QString("%1_Ks%2")
                                         .arg(matID)
                                         .arg(suffix),
                                       0,
                                       specularColor,
                                       specularTmp
                                     ));
      // ReNodeConverter::addNode(specularMap);
    }
    else {
      // if the specular map is a constant color, test if that color is too
      // bright and,if it is, lower its brightness
      if (specularTmp->getType() == TexConstant) {
        Reality::ReConstant* colTex = static_cast<Reality::ReConstant*>(specularTmp.data());
        QColor clr = colTex->getColor().convertTo(QColor::Hsl);        
        if ( clr.lightness() > RE_MAX_SPECULAR_BRIGHTNESS  ) {
          clr.setHsl(clr.hue(),clr.saturation(),RE_MAX_SPECULAR_BRIGHTNESS);
          colTex->setColor(clr);
        }
      }
      specularMap = specularTmp;
    }
  }
  else {
    // If there is no specular map then we need to dim the specular color 
    // to 1/4 of what is in the host because Lux needs much dimmer colors
    dimColor(specularColor,0.25f);
    specularMap = ReTexturePtr(new Reality::ReConstant(
                                     QString("%1_Ks%2").arg(matID).arg(suffix),
                                     0,
                                     specularColor
                                   ));
  }
  ReNodeConverter::addNode(specularMap);
  return specularMap;
}

ReMaterialPtr ReGeometryObject::convertMaterial( const QString matID, 
                                                 const QString srcMaterial,
                                                 const ReMaterialType materialType ) 
{
  QJson::Parser json;
  bool ok;

  // Convert the JSON text with the original data retrieved from the 
  // hosting app
  QVariantMap srcMat = json.parse(srcMaterial.toUtf8(), &ok).toMap();
  if (!ok) {
    RE_LOG_WARN() << "Error: invalid syntax in the data for material " << matID.toStdString();
    ReMaterialPtr matPtr = ReMaterialPtr(
      new ReMatte(matID, this)
    );
    return matPtr;
  }
  return convertMaterial(matID, srcMat, materialType);
}

ReMaterialPtr ReGeometryObject::convertMaterial( const QString matID, 
                                                 const QVariantMap& srcMat,
                                                 const ReMaterialType materialType )
{
  // Is this object a mesh light? The light flag can be enabled by the
  // <addObject> method if the object name begins with "RealityLight"
  if ( isLightFlag || matID.startsWith(REALITY_LIGHT_PREFIX) ) {
    ReLightMaterialPtr matPtr = ReLightMaterialPtr( new ReLightMaterial(matID, this) );
    // We don't want to make it possible to return the meshlight to a material,
    // it makes no sense.
    matPtr->getLight()->setFromMaterial(false);
    return matPtr;    
  }
  // Reset all the maps to a clean state
  matInfo.init();

  matInfo.shaderSource = srcMat["source"].toString();

  ReNodeConverter::init(srcMat[RE_MAT_KEY_NODES_ROOT].toMap());

  // Gather the colors first
  matInfo.diffuseColor      = convertFloatColor(
                                srcMat[RE_MAT_KEY_DIFFUSE].toMap()["color"].toList()
                              );
  matInfo.diffuse2Color     = convertFloatColor(
                                srcMat[RE_MAT_KEY_DIFFUSE2].toMap()["color"].toList()
                              );
  matInfo.specularColor     = convertFloatColor(
                                srcMat[RE_MAT_KEY_SPECULAR].toMap()["color"].toList()
                              );
  matInfo.translucenceColor = convertFloatColor(
                                srcMat[RE_MAT_KEY_TRANSLUCENCE].toMap()["color"].toList()
                              );
  matInfo.ambientColor      = convertFloatColor(
                                srcMat[RE_MAT_KEY_AMBIENT].toMap()["color"].toList()
                              );
  matInfo.coatColor         = convertFloatColor(
                                srcMat[RE_MAT_KEY_COAT].toMap()["color"].toList()
                              );

  // Check if the material type has been set
  QString srcMatType = srcMat["type"].toString();
  if (srcMatType != RE_MAT_TYPE_CODE_UNDEFINED) {
    matInfo.matTypeHint = ReMaterial::typeFromShortName(srcMatType);
  }

  // Save the possible preset for later use
  matInfo.preset = srcMat[RE_MAT_KEY_PRESET].toString();

  // Skin flag
  matInfo.isSkin = srcMat[RE_MAT_KEY_IS_SKIN].toBool();
  matInfo.materialName = srcMat["name"].toString();
  // Now let's build the texture network from the nodes that have been converted
  QString mapStr = srcMat[RE_MAT_KEY_DIFFUSE].toMap()["map"].toString();
  if (mapStr != "") {
    ReTexturePtr diffuseTmp = ReNodeConverter::convertNode(mapStr, ReNodeConverter::ColorNode);
    // Make sure that we use gain at 1.0 for human figures textures
    if (isHumanFigure) {
      auto skinMap = findImageMapTexture(diffuseTmp);
      if (!skinMap.isNull()) {
        if (skinMap->getNamedValue("gain").toFloat() < 1.0) {
          skinMap->setNamedValue("gain",1.0);
        }
      }
    }    
    // If we have a diffuse color different than pure white then we need
    // to create a diffuse texture that is a mix of the color and the texture
    if (!isPureWhite(matInfo.diffuseColor)) {
      // We use a ColorMath node because it has a built-in multiplication
      // of a color with another texture, similarly to what is in Poser
      matInfo.diffuseMap = ReTexturePtr(new Reality::ReColorMath(
                                              QString("%1_diffuse").arg(matID),
                                              0,
                                              matInfo.diffuseColor,
                                              diffuseTmp 
                                            )
                           );
    }
    else {
      // If the diffuse color is pure white then we don't need it, we can
      // safely just include the image map
      matInfo.diffuseMap = diffuseTmp;
    }
  }
  else {
    matInfo.diffuseMap = ReTexturePtr(new Reality::ReConstant(
                                            QString("%1_diffuse").arg(matID),
                                            0,
                                            matInfo.diffuseColor
                                          )
                         );
  }

  // Diffuse 2
  mapStr = srcMat[RE_MAT_KEY_DIFFUSE2].toMap()["map"].toString();
  if (mapStr != "") {
    // If the Alternate Diffuse input is present we add it to the diffuse node
    ReTexturePtr d2Tmp = ReNodeConverter::convertNode(mapStr, ReNodeConverter::ColorNode);
    // Make sure that we use gain at 1.0 for human figures textures
    if (isHumanFigure) {
      auto skinMap = findImageMapTexture(d2Tmp);
      if (!skinMap.isNull()) {
        if (skinMap->getNamedValue("gain").toFloat() < 1.0) {
          skinMap->setNamedValue("gain",1.0);
        }
      }
    }    

    // If we have a diffuse2 color different than pure white then we need
    // to create a diffuse2 texture that is a mix of the color and the texture
    if (!isPureWhite(matInfo.diffuse2Color)) {
      // We use a ColorMath node because it has a built-in multiplication
      // of a color with another texture, similarly to what is in Poser
      matInfo.diffuse2Map = ReTexturePtr(new Reality::ReColorMath(
                                              QString("%1_diffuse2").arg(matID),
                                              0,
                                              matInfo.diffuse2Color,
                                              d2Tmp 
                                            )
                            );
    }
    else {
      matInfo.diffuse2Map = d2Tmp;
    }

    // Combine the diffuse and alternate diffuse only if the diffuse is 
    // not a solid pure white or pure black color
    if (!isPureWhite(matInfo.diffuseMap) && !isPureBlack(matInfo.diffuseMap))  {
      matInfo.diffuseMap = ReColorMathPtr( new ReColorMath(
                                                 QString(
                                                   "%1_diffuse1_2::mixer"
                                                 ).arg(matID),
                                                 0,
                                                 matInfo.diffuseMap, 
                                                 matInfo.diffuse2Map,
                                                 ReColorMath::add
                                           ));
    }
    else {
      // If the diffuse channel has pure white or pure black then we simply
      // use the alternate diffuse as the diffuse
      matInfo.diffuseMap.clear();
      matInfo.diffuseMap = matInfo.diffuse2Map;
    }
  }

  // If we don't have a diffuse map or a diffuse2 map then create a texture 
  // with the diffuse color 
  if (matInfo.diffuseMap.isNull()) { 
    if ( matInfo.diffuse2Map.isNull()) {
      // Replace pure colors with more realistic ones
      if (isPureWhite(matInfo.diffuseColor)) {
        matInfo.diffuseColor = RE_WHITE_COLOR;
      }
      else if (isColorBlack(matInfo.diffuseColor)) {
        matInfo.diffuseColor = RE_BLACK_COLOR;
      }
      matInfo.diffuseMap = ReTexturePtr( 
                              new Reality::ReConstant(
                                    QString("%1_diffuse").arg(matID),
                                    0,
                                    matInfo.diffuseColor
                                  ) 
                           );
    }
    else {
      matInfo.diffuseMap = matInfo.diffuse2Map;
    }
  };

  /*
   * Specular
   */
  matInfo.uGlossiness = (int)((1.0 - srcMat[RE_MAT_KEY_UROUGHNESS].toDouble()) * 10000);
  matInfo.vGlossiness = (int)((1.0 - srcMat[RE_MAT_KEY_VROUGHNESS].toDouble()) * 10000);
  QString glossTex = srcMat[RE_MAT_KEY_VROUGHNESS_MAP].toString();

  // adjust the specular value down to account for biased vs unbiased difference
  matInfo.uGlossiness *= RE_SPECULAR_CORRECTION_FACTOR;
  matInfo.vGlossiness *= RE_SPECULAR_CORRECTION_FACTOR;

  if (matInfo.uGlossiness == 0) {
    matInfo.uGlossiness = RE_MIN_GLOSSINESS;
  }
  if (matInfo.vGlossiness == 0) {
    matInfo.vGlossiness = RE_MIN_GLOSSINESS;
  }

  auto specularTex = convertSpecular(srcMat[RE_MAT_KEY_SPECULAR].toMap(), matID,"1");
  // Should we use the specular map or specular 2?
  QVariantMap spec2 = srcMat[RE_MAT_KEY_SPECULAR2].toMap();
  QColor spec2Clr = convertFloatColor(spec2["color"].toList());
  ReTexturePtr specular2Tex;
  if ( spec2.value("map") != "" || !isColorBlack(spec2Clr) ) {
    specular2Tex = convertSpecular(spec2, matID,"2");
    matInfo.specularMap = ReColorMathPtr( 
                            new Reality::ReColorMath(
                                  QString("%1_CombinedSpec").arg(matID), 
                                  0,
                                  specularTex,
                                  specular2Tex,
                                  ReColorMath::multiply
                                )
                          );
  }
  else {
    matInfo.specularMap = specularTex;
  }

  // Convert the glossiness strength
  if (!glossTex.isEmpty()) {
    matInfo.glossinessMap = ReNodeConverter::convertNode(glossTex, ReNodeConverter::NumericNode);
    ReNodeConverter::addNode(matInfo.glossinessMap);
  }

  // Coat layer
  mapStr = srcMat[RE_MAT_KEY_COAT].toMap()["map"].toString();
  if (mapStr != "") {
    auto tmpCoat = ReNodeConverter::convertNode(
                     mapStr, ReNodeConverter::NumericNode, true
                   );
    tmpCoat->setTextureDataType(ReTexture::color);

    // If we have a coat color different than pure white then we need
    // to create a texture that is a mix of the color and the map
    if (!isPureWhite(matInfo.coatColor)) {
      matInfo.coatMap = ReTexturePtr(new Reality::ReColorMath(
                                              QString("%1_Ka").arg(matID),
                                              0,
                                              matInfo.coatColor,
                                              tmpCoat 
                                            )
                           );
    }
    else {
      matInfo.coatMap = tmpCoat;
    }
    // ReNodeConverter::addNode(matInfo.coatMap);
  }
  else {
    matInfo.coatMap = ReTexturePtr(new Reality::ReConstant(
                                            QString("%1_Ka").arg(matID),
                                            0,
                                            matInfo.coatColor
                                          )
                         );
  }

  // Translucence
  auto translucenceData = srcMat[RE_MAT_KEY_TRANSLUCENCE].toMap();
  matInfo.translucenceEnabled = translucenceData["on"].toBool();

  if (matInfo.translucenceEnabled) {    
    mapStr = translucenceData["map"].toString();
    if (mapStr != "") {
      auto tmpTrans = ReNodeConverter::convertNode(
                       mapStr, ReNodeConverter::NumericNode, true
                     );
      tmpTrans->setTextureDataType(ReTexture::color);

      // If we have a translucense color different than pure white then we need
      // to create a texture that is a mix of the color and the map
      if (!isPureWhite(matInfo.translucenceColor)) {
        matInfo.translucenceMap = ReTexturePtr(new Reality::ReColorMath(
                                                QString("%1_Kt").arg(matID),
                                                0,
                                                matInfo.translucenceColor,
                                                tmpTrans 
                                              )
                                  );
      }
      else {
        matInfo.translucenceMap = tmpTrans;
      }
      // ReNodeConverter::addNode(matInfo.translucenceMap);
    }
    else {
      matInfo.translucenceMap = ReTexturePtr(new Reality::ReConstant(
                                              QString("%1_Kt").arg(matID),
                                              0,
                                              matInfo.translucenceColor
                                            )
                                );
    }
  }

  // Alpha map
  mapStr = srcMat[RE_MAT_KEY_ALPHA].toMap()["map"].toString();
  matInfo.alphaStrength = srcMat[RE_MAT_KEY_ALPHA].toMap()["strength"].toDouble();
  if (mapStr != "") {
    // True third parameter makes the texture unique, not shared
    matInfo.alphaMap = ReNodeConverter::convertNode(mapStr, ReNodeConverter::NumericNode, true);
    matInfo.alphaMap->setTextureDataType(ReTexture::numeric);
    ReNodeConverter::addNode(matInfo.alphaMap);
  }

  // Bump map
  QVariantMap tmpMap = srcMat["bump"].toMap();
  mapStr = tmpMap["map"].toString();
  if (mapStr != "") {
    // True third parameter makes the texture unique, not shared
    matInfo.bumpMap = ReNodeConverter::convertNode(
      mapStr, ReNodeConverter::NumericNode, true
    );
    matInfo.bmStrength = tmpMap["strength"].toDouble();
    matInfo.bmPositive = tmpMap["pos"].toDouble();
    matInfo.bmNegative = tmpMap["neg"].toDouble();
    ReNodeConverter::addNode(matInfo.bumpMap);
  }
  // Displacement map
  tmpMap = srcMat[RE_MAT_KEY_DISPLACEMENT].toMap();
  mapStr = tmpMap["map"].toString();
  if (mapStr != "") {
    matInfo.displacementMap = ReNodeConverter::convertNode(mapStr, ReNodeConverter::NumericNode);
    matInfo.dmStrength = tmpMap["strength"].toDouble();
    matInfo.dmPositive = tmpMap["pos"].toDouble();
    matInfo.dmNegative = tmpMap["neg"].toDouble();
    ReNodeConverter::addNode(matInfo.displacementMap);
  }

  // Ambient settings
  tmpMap = srcMat[RE_MAT_KEY_AMBIENT].toMap();
  mapStr = tmpMap["map"].toString();
  if (!isColorBlack(matInfo.ambientColor) && 
      (srcMat[RE_MAT_KEY_LIGHT_GAIN].toFloat() > 0.0) ) 
  {
    // Disabled by default
    // matInfo.isLight = true;
    matInfo.lightAlpha = srcMat[RE_MAT_KEY_LIGHT_ALPHA].toBool();
    matInfo.lightGain = srcMat[RE_MAT_KEY_LIGHT_GAIN].toFloat();

    // We set a texture only if there is a map. Ambient color is not used
    // for Reality/Lux because it doesn't work well. The texture from Diffuse
    // is used for the emission.
    if (mapStr != "") {
      ReTexturePtr tmpTex = ReNodeConverter::convertNode(mapStr, ReNodeConverter::ColorNode);

      // We use a ColorMath node because it has a built-in multiplication
      // of a color with another texture, similarly to what is in Poser
      matInfo.ambientMap = ReTexturePtr(new Reality::ReColorMath(
                                              QString("%1_emitter").arg(matID),
                                              0,
                                              matInfo.ambientColor,
                                              tmpTex 
                                            )
                            );
      matInfo.ambientMap->setNamedValue("function", ReColorMath::none);
      ReNodeConverter::addNode(matInfo.ambientMap);
    }
  }

  /*
   * Here is where we create the actual materials
   */
  // This call must be executed before createMaterial() because the 
  // original shader data must be in the database before createMaterial() can do 
  // its job.
  storeOriginalShader(getInternalName(), matID, srcMat);
  ReMaterialPtr matPtr;
  createMaterial(matID, matPtr, materialType);

  return matPtr;
}

/**
 * Function to test a few simple conditions that mark the material as being Matte
 */
bool isMatteMaterial( ReMaterialInfo& matInfo  ) {
  // Diffuse and specular are the same 
  if (matInfo.diffuseMap == matInfo.specularMap ) {
    return true;
  }
  // Glossiness is less or equal to 0.5 and there is no alt spec. map.
  if ( matInfo.uGlossiness == matInfo.vGlossiness && matInfo.uGlossiness <= 0.5 ) {
    return true;
  }
  // Specular color is black, within tollerances, and the specular map is the same 
  // texture used for the diffuse map. Or, the specular map is a simple color.
  if ( isColorBlack(matInfo.specularColor) && 
       ( matInfo.specularMap.isNull() || (matInfo.specularMap->getType() == TexConstant) )
     ) 
  {
    return true;
  }
  return false;
}

void setModifiers( DisplaceableMaterial* mat, ReMaterialInfo& matInfo ) {
  if (!matInfo.bumpMap.isNull()) {    
    mat->setBumpMap(matInfo.bumpMap);
    mat->setBmPositive(matInfo.bmPositive);
    mat->setBmNegative(matInfo.bmNegative);
    mat->setBmStrength(matInfo.bmStrength);
/*
    // If the bump map is a normal map then we set the strength at
    // 100 as a starting point.
    auto im = findImageMapTexture(matInfo.bumpMap);
    if (!im.isNull() && im->getNamedValue("normalMap").toBool()) {
      mat->setBmStrength(1.0);
    }
    */
  }  
  if (!matInfo.displacementMap.isNull()) {
    mat->setDisplacementMap(matInfo.displacementMap);
    mat->setDmPositive(matInfo.dmPositive);
    mat->setDmNegative(matInfo.dmNegative);
    mat->setDmStrength(matInfo.dmStrength);
  }  
  if (!matInfo.ambientMap.isNull()) {
    mat->setAmbientMap(matInfo.ambientMap);
  }  
}

ReMaterialType ReGeometryObject::createMaterialFromAcselShader( 
                 const QString& matID, 
                 ReMaterialPtr& matPtr 
               )
{
  // Let's be completely deterministic
  matInfo.foundAcselDefaultShader = false;
  // Get the reference to ACSEL
  ReAcsel* acsel = ReAcsel::getInstance();
  // Get the ACSEL ID based on all the textures used... 
  QString acselID = computeAcselID(matID, false);
  matInfo.acselID = acselID;

  QVariantMap shaderInfo;
  if (!acsel->findShader( acselID, shaderInfo ) ) {
    // Then try to find a generic shader
    QString defaultAcselID = computeAcselID(matID, true);
    if (!acsel->findShader( defaultAcselID, shaderInfo ) ) {
      return MatUndefined;
    }
    matInfo.acselID = defaultAcselID;
    matInfo.foundAcselDefaultShader = true;
  }
  matInfo.acselSetID = shaderInfo["SetID"].toString();
  // convert the JSON Data to a QVariantMap
  QJson::Parser parser;
  bool ok;

  QVariantMap shader = parser.parse(
                         shaderInfo["ShaderCode"].toString().toUtf8(), &ok
                       )
                       .toMap();
  //! This should never happen. Just in case...
  if (!ok) {
    return MatUndefined;
  }
  ReMaterialType matType = ReMaterial::typeFromName(shader.value("type").toString());

  // We just set the type of the material if the "typeOnly" field exists and
  // if if contains the string identifying the renderer that created the shader
  // that we are converting.
  QString typeOnlyFlag = shader.value("typeOnly", "").toString();
  if (!typeOnlyFlag.isEmpty()) {
    matInfo.setMaterialTypeOnly = (typeOnlyFlag.indexOf(matInfo.shaderSource) != -1);
  }
  // If we have found a default ACSEL shader then we don't have the material
  // data at this point. We must let the rest of the material processing 
  // continue and *then*
  // apply the settings from the ACSEL default shader. So we exit at this point
  // and let the information in matInfo drive the rest of the conversion.
  if (matInfo.foundAcselDefaultShader) {
    matInfo.acselShader = (matInfo.setMaterialTypeOnly ? QVariantMap() : shader);
    return matType;
  }

  matPtr = ReMaterialPtr( 
    MaterialCreator::createMaterial(matType, this, matID)
  );
  auto matImporter = ReQtMaterialImporterFactory::getImporter(matType);
  matImporter->importFromClipboard(matPtr, shader, ReQtMaterialImporter::Replace);
  matPtr->setAcselID(acselID);
  matPtr->setAcselSetName(shaderInfo["SetName"].toString());
  matPtr->setAcselSetID(shaderInfo["SetID"].toString());
  matPtr->setEdited(false);

  // Check if we need to restore an associated volume
  QString innerVolume = matPtr->getInnerVolume();
  if ( !innerVolume.isEmpty() ) {
    QString volCode;
    if ( acsel->findVolume( shaderInfo["SetID"].toString(),
                            shaderInfo["VolumeID"].toInt(),
                            volCode) )
    {
      // Interpret the volume code...
      QVariantMap volData = parser.parse( volCode.toUtf8(), &ok ).toMap();
      if (ok) {
        ReQtVolumeImporter volumeImporter;
        volumeImporter.importVolume( 
          RealitySceneData->getVolume(innerVolume), volData
        );
      }
    }
  }
  return matType;
}

bool textureIsNotWhite( const ReTexturePtr tex ) {
  if ( tex->getType() == TexConstant ) {
    return !isPureWhite(tex.staticCast<ReConstant>()->getColor());
  }
  return true;
}

void ReGeometryObject::convertIRayTranslucency( Glossy* mat, 
                                                ReTexturePtr t ) 
{
  // This transformation makes sense only if the texture is of type
  // ReColorMath.
  if (t->getType() != TexColorMath) {
    return;
  }

  ReColorMathPtr transTex = t.staticCast<ReColorMath>();
  auto matName = mat->getName();
  auto diffTex = mat->getKd();
  ReMixTexture* newDiff = static_cast<ReMixTexture*>(
                            TextureCreator::createTexture(
                              QString("%1_diffMixer").arg(matName), 
                              TexMix,
                              mat,
                              ReTexture::color
                            )
                          );
  newDiff->setTexture1(diffTex);
  mat->setKd(ReTexturePtr(newDiff));
  auto t2 = static_cast<ReConstant*>(
              TextureCreator::createTexture(
                QString("%1_diffMixer_t2").arg(matName), 
                TexConstant,
                mat,
                ReTexture::color
              )
            );
  t2->setColor(transTex->getColor1());
  newDiff->setTexture2(ReTexturePtr(t2));
  newDiff->setMixTexture(ReTexturePtr(
                           TextureCreator::createTexture(
                             QString("%1_diffMixer_driver").arg(matName),
                             transTex->getTexture1()
                           )));
}

void changeTextureGain( ReTexturePtr t ) {
  t->setNamedValue("gain", 1.1);
}

void ReGeometryObject::createMaterial( const QString matID, 
                                       ReMaterialPtr& matPtr,
                                       const ReMaterialType materialType ) 
{
  // Now let's see if this material is of a predetermined type...
  ReMaterialType matClass = materialType;

  bool visibleInRender = true;
  if (matClass == MatUndefined) {
    // Is this a Water prop?
    if (matID == REALITY_WATER_MAT) {
      matClass = MatWater;
    }
    // Hide the materials that are used for some of the Reality props
    else if (matID == REALITY_LIGHT_BACK_MATERIAL || 
             matID == REALITY_LIGHT_HANDLE_MATERIAL ||
             matID == REALITY_IBL_SPHERE_MAT
            ) 
    {
      matClass = MatMatte;
      visibleInRender = false;
    }
    // This is not inside a else clause because ACSEL must be given
    // the opportunity to override whatever conversion hint was found
    // before, if an ACSEL shader is available.
    ReMaterialType axlMatClass;
    if ( (axlMatClass = createMaterialFromAcselShader(matID, matPtr)) != MatUndefined ) {
      matClass = axlMatClass;
      // If the material has been created from an ACSEL shader then 
      // we are done. Get outta here!
      if (!matInfo.foundAcselDefaultShader) {
        return;
      }
    }
  }

  if (matClass == MatUndefined) {
    if (matInfo.isSkin) {
      matClass = MatSkin;
    }
    else {
      // At this point matClass is still MatUndefined
      if (matInfo.matTypeHint != MatUndefined) {
        matClass = matInfo.matTypeHint;
      }
      else if ( matID.contains("glass", Qt::CaseInsensitive) ) {
        matClass = MatGlass;
      }
      else {
        // If the specular maps is present then it's a Glossy mat
        if (!matInfo.specularMap.isNull()) {
          matClass = MatGlossy;
        }
        else {
          if (matClass == MatUndefined && isMatteMaterial(matInfo)) {
            matClass = MatMatte;
          }
          else {
            matClass = MatGlossy;
          }
        }
      }
    }
  }

  switch(matClass) {
    case MatCloth: {
      ReCloth* newMat = new Reality::ReCloth(matID, this);
      setModifiers(newMat, matInfo);
      newMat->setAlphaStrength(matInfo.alphaStrength);
      if (!matInfo.alphaMap.isNull()) {
        newMat->setAlphaMap(matInfo.alphaMap);
      }
      if (!matInfo.preset.isNull()) {
        if (matInfo.preset == "silk") {
          newMat->setNamedValue("preset",RE_CLOTH_SILK_CHARMEUSE);
        }
      }
      matPtr = ReMaterialPtr(newMat);
      break;
    }
    case MatUndefined:
    case MatGlossy: {
      Glossy* newMat = new Reality::Glossy(matID, this);
      newMat->setKd(matInfo.diffuseMap);
      newMat->setKs(matInfo.specularMap);
      if ((matInfo.coatMap->getType() != TexConstant) || 
           textureIsNotWhite(matInfo.coatMap)) 
      {
        newMat->setKa(matInfo.coatMap);
        newMat->setTopCoat(true);
      }
      if (matInfo.shaderSource == RE_HOST_RENDERER_CODE_IRAY) {        
        if (matInfo.translucenceEnabled) {
          convertIRayTranslucency(newMat, matInfo.translucenceMap);
        }    
      }
      else {
        if (matInfo.translucenceEnabled) {
          newMat->setKt(matInfo.translucenceMap);
          newMat->setTranslucent(true);
        }
      }
      newMat->setUGlossiness(matInfo.uGlossiness);
      newMat->setVGlossiness(matInfo.vGlossiness);
      if (!matInfo.glossinessMap.isNull()) {
        newMat->setKg(matInfo.glossinessMap);
      }
      setModifiers(newMat, matInfo);
      newMat->setAlphaStrength(matInfo.alphaStrength);
      if (!matInfo.alphaMap.isNull()) {
        newMat->setAlphaMap(matInfo.alphaMap);
      }
      matPtr = ReMaterialPtr(newMat);
      break;
    }
    case MatLight: {
      ReLightMaterial* newMat = new ReLightMaterial(matID, this);
      if (!matInfo.diffuseMap.isNull()) {
        ReLightPtr matLight = newMat->getLight();
        matLight->setTexture(matInfo.diffuseMap->findImageMap());
        matLight->setAlphaChannel(false);
        matLight->setLightGroup(matID);
      }
      matPtr = ReMaterialPtr(newMat);
      break;
    }
    case MatSkin: {
      ReSkin* newMat = new Reality::ReSkin(matID, this);
      matPtr = ReMaterialPtr(newMat);
      QString objID = newMat->getParent()->getInternalName();

      newMat->setKd(matInfo.diffuseMap);
      newMat->setKs(matInfo.specularMap);
      if ((matInfo.coatMap->getType() != TexConstant) || 
           textureIsNotWhite(matInfo.coatMap)) 
      {
        newMat->setTopCoat(true);
        newMat->setKa(matInfo.coatMap);
      }
      // iRays's treatment of skin is to use the translucency channel to 
      // add a pinkish hue to the material. That doesn't work well in LuxRender,
      // while assigning the same value to the Coat channel achieves the 
      // right result. 
      if (matInfo.shaderSource == RE_HOST_RENDERER_CODE_IRAY) {
        if (matInfo.translucenceEnabled && 
            textureIsNotWhite(matInfo.translucenceMap)) {

          matInfo.coatMap = ReTexturePtr(
                              TextureCreator::createTexture(
                                QString("%1_Ka").arg(matID),
                                matInfo.translucenceMap
                              )
                            );
          newMat->setKa(matInfo.coatMap);
          newMat->setTopCoat(true);
          transformImageMaps(newMat->getKa(), changeTextureGain ) ;
          newMat->setCoatThickness(RE_SKIN_ADJUSTED_COAT_THICKNESS);
        }
      }

      newMat->setUGlossiness(matInfo.uGlossiness);
      // Anisotropic specular enforced. Tests have shown that
      // an asymmetry of 76% works well
      newMat->setVGlossiness(matInfo.uGlossiness * 0.76);
      if (!matInfo.glossinessMap.isNull()) {
        newMat->setKg(matInfo.glossinessMap);
      }
      // Adjust the fresnel value to give the skin a nice sheen
      newMat->setFresnelAmount(RE_SKIN_ADJUSTED_FRESNEL);
      setModifiers(newMat, matInfo);
      // Alpha
      newMat->setAlphaStrength(matInfo.alphaStrength);
      if (!matInfo.alphaMap.isNull()) {
        newMat->setAlphaMap(matInfo.alphaMap);
      }

      break;
    }
    case MatMatte: {
      ReMatte* newMat = new Reality::ReMatte(matID, this);
      newMat->setKd(matInfo.diffuseMap);
      newMat->setRoughness(0.4f);
      setModifiers(newMat, matInfo);
      // Alpha
      newMat->setAlphaStrength(matInfo.alphaStrength);
      if (!matInfo.alphaMap.isNull()) {
        newMat->setAlphaMap(matInfo.alphaMap);
      }
      matPtr = ReMaterialPtr(newMat);
      break;
    }
    case MatGlass: {
      ReGlass* newMat = new Reality::ReGlass(matID, this);
      newMat->setKt(matInfo.diffuseMap);
      newMat->setKr(
        ReTexturePtr(
          new Reality::ReConstant(QString("%1_Kr").arg(matID), newMat)
        )
      );
      setModifiers(newMat, matInfo);
      newMat->setAlphaStrength(1.0);
      // Check if a preset has been specified
      if (!matInfo.preset.isNull()) {
        if (matInfo.preset == "architectural") {
          newMat->setNamedValue("glassType", ReGlass::ArchitecturalGlass);
        }
        else if (matInfo.preset == "frosted") {
          newMat->setNamedValue("glassType", ReGlass::FrostedGlass);
        }
      }
      matPtr = ReMaterialPtr(newMat);      
      break;
    }
    case MatMetal: {
      ReMetal* metal = new Reality::ReMetal(matID, this);
      metal->setKr(matInfo.diffuseMap);
      metal->setHPolish(std::max(RE_METAL_DEFAULT_POLISH,matInfo.uGlossiness));
      metal->setVPolish(std::max(RE_METAL_DEFAULT_POLISH,matInfo.vGlossiness));
      // Alpha
      metal->setAlphaStrength(matInfo.alphaStrength);
      if (!matInfo.alphaMap.isNull()) {
        metal->setAlphaMap(matInfo.alphaMap);
      }
      if (!matInfo.preset.isNull()) {
        if (matInfo.preset == "aluminum") {
          metal->setNamedValue("metalType", ReMetal::Aluminum);
        }
        else if (matInfo.preset == "steel") {
          metal->setNamedValue("metalType", ReMetal::Nickel);
        }
      }
      setModifiers(metal, matInfo);
      matPtr = ReMaterialPtr( metal );
      break;
    }    
    case MatMirror: {
      ReMirror* mirror = new Reality::ReMirror(matID, this);
      mirror->setKr(matInfo.diffuseMap);
      setModifiers(mirror, matInfo);
      matPtr = ReMaterialPtr( mirror );
      break;
    }        
    case MatVelvet: {
      ReVelvet* velvet = new Reality::ReVelvet(matID, this);
      velvet->setKd(matInfo.diffuseMap);
      setModifiers(velvet, matInfo);
      matPtr = ReMaterialPtr( velvet );
      break;
    }        
    case MatWater: {
      ReWater* water = new Reality::ReWater(matID, this);
      water->setKt(matInfo.diffuseColor);
      matPtr = ReMaterialPtr( water );      
      break;
    }
    case MatNull: {
      ReNull* nullMat = new Reality::ReNull(matID, this);
      nullMat->setOuterVolume(RE_AIR_VOLUME);
      matPtr = ReMaterialPtr( nullMat );
      break;
    }
    case MatHair: 
      //!TODO! FINISH THIS!!!!!!
      break;
    case MatMix: 
      //!TODO! FINISH THIS!!!!!!
      break;
  }

  // Special case: the material emits light because the
  // the original shader was set with an ambient value and a non-black 
  // color

  // Detect if this is a hair prop. All Poser hair props start with "figureHair"
  bool isHairProp = internalName.startsWith("figureHair");

  // Do not convert the material to light if this is a hair prop
  if ( !isHairProp ) {
    if ( matInfo.isLight ) {
      matPtr->setNamedValue("emitsLight", true);
      matPtr->setNamedValue("lightGain", matInfo.lightGain);
    }
  }
  if (!visibleInRender) {
    matPtr->setVisibleInRender(visibleInRender);
  }

  // Apply the generic ACSEL shader, if present
  if (matInfo.foundAcselDefaultShader && !matInfo.setMaterialTypeOnly) {
    auto matImporter = ReQtMaterialImporterFactory::getImporter(matClass);
    matImporter->importFromClipboard(
      matPtr, matInfo.acselShader, ReQtMaterialImporter::ShallowReplace
    );
    matPtr->setAcselID(matInfo.acselID);
    matPtr->setAcselSetID(matInfo.acselSetID);
    matPtr->setAcselSetName(RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL);
    matPtr->setEdited(false);    
  }
  matPtr->setAcselID(matPtr->computeAcselID());
}

void ReGeometryObject::addMaterial( const QString matID, const QString srcMat) {
  // Convert the JSON text with the original data retrieved from the 
  // hosting app
  QJson::Parser json;
  bool ok;
  QVariantMap matInfo = json.parse(srcMat.toUtf8(), &ok).toMap();
  if (!ok) {
    RE_LOG_WARN() << "Error: invalid syntax in the data for material " << matID.toStdString();
    return;
  }

  addMaterial(matID, matInfo);
}

void ReGeometryObject::addMaterial( const QString matID, 
                                    const QVariantMap& srcMat) 
{
  if (materials.contains(matID)) {
    RE_LOG_INFO() << QString("Material %1/%2 already existing. Addition skipped.")
                       .arg(name)
                       .arg(matID).toStdString();
    return;
  }
  ReMaterialPtr newMat = convertMaterial(matID, srcMat);
  if (newMat.isNull()) {
    RE_LOG_WARN() << "Error: material " << matID.toStdString() << " could not be converted";
    return;
  }
  materials[matID] = newMat;
  // If this is a material converted to light then we need to add it to the list
  // of lights as well
  if (newMat->getType() == MatLight) {
    lights->insert(matID, newMat);
    ReLightPtr matLight = newMat.staticCast<ReLightMaterial>()->getLight();
    // Add the light to the list of lights of the scene
    RealitySceneData->saveLight(matLight->getID(), matLight);
    RealitySceneData->notifyGUI("lightAdded", matLight->getID());
  }
}


void ReGeometryObject::updateMaterial( const QString matName, 
                                       const QString srcMat) 
{
  if (materials.contains(matName)) {
    materials[matName].clear();
    materials[matName] = convertMaterial(matName,srcMat);
  }
}

void ReGeometryObject::updateMaterial( const QString matName, 
                                       const QVariantMap srcMat) 
{
  if (materials.contains(matName)) {
    materials[matName].clear();
    materials[matName] = convertMaterial(matName,srcMat);
  }
}

void ReGeometryObject::changeMaterialType( const QString materialID, 
                                           const QString jsonData, 
                                           const ReMaterialType newType) 
{
  QJson::Parser json;
  bool ok;

  // Convert the JSON text with the original data retrieved from the 
  // hosting app
  QVariantMap srcMat = json.parse(jsonData.toUtf8(), &ok).toMap();
  if (!ok) {
    RE_LOG_WARN() << "Error: invalid syntax in the data for material " 
                  << materialID.toStdString();
    return;
  }
  changeMaterialType(materialID, srcMat, newType);
}

void ReGeometryObject::changeMaterialType( const QString materialID, 
                                           const QVariantMap& srcData, 
                                           const ReMaterialType newType ) 
{

  if (materials.contains(materialID)) {
    ReMaterialType oldType = materials.value(materialID)->getType();
    // If this is a re-conversion from Light back to the original mat, then 
    // we need to delete the light associated with it.
    if (oldType == MatLight) {
      ReLightMaterialPtr lm = materials.value(materialID).staticCast<ReLightMaterial>();
      QString lightID = lm->getLight()->getID();
      RealitySceneData->deleteLight(lightID);
      deleteLight(materialID);
    }

    ReMaterialPtr newMat = convertMaterial(materialID, srcData, newType);
    newMat->setAcselID(materials[materialID]->getAcselID());
    // Delete the old material
    materials[materialID].clear();
    // Store the new one
    materials[materialID] = newMat;

    if (newMat->getType() == MatLight) {
      newMat.staticCast<ReLightMaterial>()->setOriginalType(oldType);
      lights->insert(materialID, newMat);
      ReLightPtr matLight = newMat.staticCast<ReLightMaterial>()->getLight();
      matLight->setFromMaterial(true);

      // Add the light to the list of lights of the scene
      RealitySceneData->saveLight(matLight->getID(), matLight);
    }
  }  
}

void ReGeometryObject::changeMaterialType( const QString materialID, 
                                           const ReMaterialType newType) {
  if (!materials.contains(materialID)) {
    RE_LOG_WARN() << "Material " << materialID.toStdString() 
                  << " cannot be converted because it does not exist";
    return;
  }

  // Make sure to not create another smart pointer for the old material...
  auto oldMat = materials[materialID].data();
  auto newMat = ReMaterialPtr(
                  MaterialCreator::createMaterial(newType, this, materialID)
                );

  ReMaterialType oldType = oldMat->getType();
  // If this is a re-conversion from Light back to the original mat, then 
  // we need to delete the light associated with it.
  if (oldType == MatLight) {
    ReLightMaterialPtr lm = materials.value(materialID).staticCast<ReLightMaterial>();
    QString lightID = lm->getLight()->getID();
    RealitySceneData->deleteLight(lightID);
    deleteLight(materialID);
  }

  // If this is a material converted to light then we need to add it to the list
  // of lights as well
  if (newMat->getType() == MatLight) {
    newMat->fromMaterial(oldMat);
    newMat.staticCast<ReLightMaterial>()->setOriginalType(oldType);
    lights->insert(materialID, newMat);
    ReLightPtr matLight = newMat.staticCast<ReLightMaterial>()->getLight();
    matLight->setFromMaterial(true);

    // Add the light to the list of lights of the scene
    RealitySceneData->saveLight(matLight->getID(), matLight);  
  }
  else {
    newMat->fromMaterial(oldMat);
    newMat->setAcselID(materials[materialID]->getAcselID());
  }
  materials[materialID].clear();
  materials[materialID] = newMat;
}


// The data QVariantMap is a map of the single material to be restored
void ReGeometryObject::restoreMaterial( const QString matID, const QVariantMap& data ) {
  if (materials.contains(matID)) {
    materials[matID].clear();
  }
  // This should never happen...
  if ( data.value("name") != matID ) {
    RE_LOG_DEBUG() << "Error: in restoreMaterial() matID " << QSS(matID)
                   << " different from material name: "    << QVS(data.value("name"));
    return;
  }
  QString matTypeStr = data.value("type").toString(); //.toLower();
  ReMaterialType matType = ReMaterial::typeFromName(matTypeStr);

  ReMaterialPtr newMat = ReMaterialPtr(
    MaterialCreator::createMaterial(matType, this, matID)
  );
  materials[matID] = newMat;
  // If this is a material converted to light then we need to add it to the list
  // of lights as well
  if (matType == MatLight) {
    lights->insert(matID, newMat);
  }

  auto matImporter = ReQtMaterialImporterFactory::getImporter(matType);
  matImporter->importMaterial(newMat, data);
}

bool ReGeometryObject::isLightEmitter() {
  return lights->count() > 0;
}

void ReGeometryObject::serializeMaterialLights( QDataStream& dataStream ) {
  // Serialize the lights. Since the light list shares the same
  // pointers to the materials stored in the materials list, 
  // we just need to serialize the keys and then recreate the references
  // from the keys, once we deserialize
  dataStream << lights->count();
  ReMaterialIterator il(*lights);
  while( il.hasNext() ) {
    il.next();
    dataStream << il.key();
  }
}

void ReGeometryObject::serialize( QDataStream& dataStream ) {
  dataStream << name              << internalName 
             << geometryFile      << visible
             << materials.count() << instanceSourceID;

  // Serialize the material
  ReMaterialIterator i(materials);
  while( i.hasNext() ) {
    i.next();
    i.value()->serialize(dataStream);
  }
  
  serializeMaterialLights(dataStream);
}

void ReGeometryObject::deserializeMaterialLights( QDataStream& dataStream ) {
  // Number of light materials
  int numLights;

  // The stream is expected to be positioned before the number of lights 
  dataStream >> numLights;
  // Retrieve the material lights
  for (int i = 0; i < numLights; ++i) {
    QString key;
    dataStream >> key;
    ReLightMaterialPtr matLight = materials.value(key).staticCast<ReLightMaterial>();
    lights->insert(key, matLight);
    RealitySceneData->saveLight(matLight->getLight()->getID(), matLight->getLight());
  }  
}

// Static method
ReGeometryObject* ReGeometryObject::deserialize( QDataStream& dataStream ) {

  QString _name, 
          _internalName, 
          _geometryFile,
          _instanceSource;
  bool _visible;

  // Number of materials
  int numMaterials;

  dataStream >> _name         >> _internalName 
             >> _geometryFile >> _visible 
             >> numMaterials  >> _instanceSource;

  ReGeometryObject* newObj = new ReGeometryObject(_name, _internalName, _geometryFile);
  newObj->setVisible(_visible);
  newObj->setInstanceSourceID(_instanceSource);
  for (int i = 0; i < numMaterials; ++i) {
    ReMaterial* newMat = MaterialCreator::deserialize(dataStream,newObj);
    if (newMat) {
      newObj->materials[newMat->getName()] = ReMaterialPtr(newMat);
    }
  }

  // Retrieve the material lights
  newObj->deserializeMaterialLights(dataStream);
  return newObj;
}

void ReGeometryObject::deserializeMaterial( const QString& materialName, 
                                            QDataStream& dataStream ) {
  ReMaterial* newMat = MaterialCreator::deserialize(dataStream,this);
  if (newMat) {
    if (materials.contains(materialName)) {
      materials[materialName].clear();
      materials[materialName] = ReMaterialPtr(newMat);
    }
  }
}

