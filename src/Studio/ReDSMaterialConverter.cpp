/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReDSMaterialConverter.h"

#include <boost/bind/bind.hpp>
#include <QColor>
#include <dzboolproperty.h>
#include <dzcolorproperty.h>
#include <dzfloatproperty.h>
#include <dzimageproperty.h>
#include <dzmaterial.h>
#include <dzstringproperty.h>

#include "ReMaterialPropertyKeys.h"
#include "ReNodeConverter.h"
#include "ReTools.h"


namespace Reality {
  // Property names used by Studio
  const char* DS_PROP_DIFFUSE               = "Diffuse Color";
  const char* DS_PROP_DIFFUSE_STRENGTH      = "Diffuse Strength";
  const char* DS_PROP_SPECULAR              = "Specular Color";
  const char* DS_PROP_SPECULAR2             = "Specular2 Color";
  const char* DS_PROP_IR_GLOSSY_CLR         = "Glossy Color";
  const char* DS_PROP_IR_GLOSSY_LAYERED_WGT = "Glossy Layered Weight";
  const char* DS_PROP_IR_GLOSSY_GLOSSY_REFLECTIVITY = "Glossy Reflectivity";
  const char* DS_PROP_IR_TRANSLUCENCY       = "Translucency Color";
  const char* DS_PROP_SPECULAR2_ON          = "Specular2 Active";
  const char* DS_PROP_SPECULAR_STRENGTH     = "Specular Strength";
  const char* DS_PROP_SPECULAR2_STRENGTH    = "Specular2 Strength";
  const char* DS_PROP_IR_METALLIC_CLR       = "Metallic Flakes Color";

  const char* DS_PROP_U_TILE             = "Map Tiling U";
  const char* DS_PROP_U_TILE2            = "Horizontal Tiles";
  const char* DS_PROP_V_TILE             = "Map Tiling V";
  const char* DS_PROP_V_TILE2            = "Vertical Tiles";
  const char* DS_PROP_U_OFFSET           = "Horizontal Offset";
  const char* DS_PROP_V_OFFSET           = "Vertical Offset";
  const char* DS_PROP_GLOSSINESS         = "Glossiness";
  const char* DS_PROP_IR_ROUGHNESS       = "Glossy Roughness";
  const char* DS_PROP_ALPHA              = "Opacity Strength";
  const char* DS_PROP_IR_ALPHA           = "Cutout Opacity";
  const char* DS_PROP_BUMP_MAP           = "Bump Strength";
  const char* DS_PROP_NEG_BUMP           = "Bump Minimum";
  // Bump map properties are not standardized. Both versions are used
  const char* DS_PROP_NEG_BUMP2          = "Negative Bump";
  const char* DS_PROP_POS_BUMP           = "Bump Maximum";
  const char* DS_PROP_POS_BUMP2          = "Positive Bump";
  const char* DS_PROP_DISP_MAP           = "Displacement Strength";
  const char* DS_PROP_NEG_DISP           = "Minimum Displacement";
  const char* DS_PROP_NEG_DISP2          = "Displacement Minimum";
  const char* DS_PROP_POS_DISP           = "Maximum Displacement";
  const char* DS_PROP_POS_DISP2          = "Displacement Maximum";
  const char* DS_PROP_NORMAL_MAP         = "Normal Map";
  const char* DS_PROP_NORMAL_MAP_STRENGHT= "Normal Map Strength";
  const char* DS_PROP_AMBIENT_ON         = "Ambient Active";
  const char* DS_PROP_AMBIENT_COLOR      = "Ambient Color";
  const char* DS_PROP_AMBIENT_STRENGTH   = "Ambient Strength";
  const char* DS_PROP_TAGS               = "Tags";
  
  // The Reality versions of the same properties
  const char* RE_PROP_DIFFUSE         = "kd";
  const char* RE_PROP_DIFFUSE_VALUE   = "kd_value";
  const char* RE_PROP_SPECULAR        = "ks";
  const char* RE_PROP_SPECULAR_VALUE  = "ks_value";
  const char* RE_PROP_SPECULAR2       = "ks2";
  const char* RE_PROP_SPECULAR2_VALUE = "ks2_value";
  const char* RE_PROP_H_TILE          = "htile";
  const char* RE_PROP_V_TILE          = "vtile";
  const char* RE_PROP_H_OFFSET        = "hoffset";
  const char* RE_PROP_V_OFFSET        = "voffset";
  const char* RE_PROP_GLOSSINESS      = "gloss";
  const char* RE_PROP_ALPHA_MAP       = "alpha";
  const char* RE_PROP_BUMP_MAP        = "bm";
  const char* RE_PROP_BUMP_LOW        = "bm_low";
  const char* RE_PROP_BUMP_HIGH       = "bm_high";
  const char* RE_PROP_BUMP_VALUE      = "bm_value";
  const char* RE_PROP_DISP_MAP        = "dm";
  const char* RE_PROP_DISP_LOW        = "dm_low";
  const char* RE_PROP_DISP_HIGH       = "dm_high";
  const char* RE_PROP_DISP_VALUE      = "dm_value";
  const char* RE_PROP_NORMAL_MAP      = "nm";
  const char* RE_PROP_AMBIENT_COLOR   = "amb";
  const char* RE_PROP_AMBIENT_ON      = "amb_on";
  const char* RE_PROP_AMBIENT_VALUE   = "amb_value";

  const float RE_IRAY_DEFAULT_NEG_BUMP = -0.0003;
  const float RE_IRAY_DEFAULT_POS_BUMP =  0.0003;
  //! Coefficient we use to adjust the strength of bump mapping when
  //! a bump map and a normal map are mixed together.
  const float RE_BUMP_STRENGTH_CORRECTION =  0.66;

  //! iRay's glossy materials start very shiny and there is no way making them
  //! matte. So we start with a low roughness that approximates the level of
  //! shine that we see on such materials.
  const float RE_IRAY_BASE_ROUGHNESS      =  0.5;

  //! Gain used for spec maps for iRay shaders
  const float RE_IRAY_SPECMAP_ADJUSTED_GAIN =  0.5;

// Static definition for the instance
ReDSMaterialConverter* ReDSMaterialConverter::instance = 0;

// Static method
ReDSMaterialConverter* ReDSMaterialConverter::getInstance() {
  if (!instance) {
    instance = new ReDSMaterialConverter();
    // Initialize the property LUT
    initLUT();
  }
  return instance;
}
// LUT for the Studio properties to be converted
ReDSMaterialConverter::PropertyLUT_t ReDSMaterialConverter::propertyLUT;

ReDSMaterialConverter::PropertyCallback_t ReDSMaterialConverter::propertyConverters;

void ReDSMaterialConverter::initLUT() {
  // Map the DAZ property names to the Reality property names
  propertyLUT[DS_PROP_DIFFUSE]            = RE_PROP_DIFFUSE;
  propertyLUT[DS_PROP_DIFFUSE_STRENGTH]   = RE_PROP_DIFFUSE_VALUE;
  propertyLUT[DS_PROP_SPECULAR]           = RE_PROP_SPECULAR;
  propertyLUT[DS_PROP_SPECULAR_STRENGTH]  = RE_PROP_SPECULAR_VALUE;
  propertyLUT[DS_PROP_SPECULAR2]          = RE_PROP_SPECULAR2;
  propertyLUT[DS_PROP_SPECULAR2_STRENGTH] = RE_PROP_SPECULAR2_VALUE;
  propertyLUT[DS_PROP_U_TILE]             = RE_PROP_H_TILE;
  propertyLUT[DS_PROP_U_TILE2]            = RE_PROP_H_TILE;
  propertyLUT[DS_PROP_V_TILE]             = RE_PROP_V_TILE;
  propertyLUT[DS_PROP_V_TILE2]            = RE_PROP_V_TILE;
  propertyLUT[DS_PROP_U_OFFSET]           = RE_PROP_H_OFFSET;
  propertyLUT[DS_PROP_V_OFFSET]           = RE_PROP_V_OFFSET;
  propertyLUT[DS_PROP_GLOSSINESS]         = RE_PROP_GLOSSINESS;
  propertyLUT[DS_PROP_IR_ROUGHNESS]       = RE_PROP_GLOSSINESS;
  propertyLUT[DS_PROP_ALPHA]              = RE_PROP_ALPHA_MAP;
  propertyLUT[DS_PROP_IR_ALPHA]           = RE_PROP_ALPHA_MAP;
  propertyLUT[DS_PROP_BUMP_MAP]           = RE_PROP_BUMP_MAP;
  propertyLUT[DS_PROP_NEG_BUMP]           = RE_PROP_BUMP_LOW;
  propertyLUT[DS_PROP_NEG_BUMP2]          = RE_PROP_BUMP_LOW;
  propertyLUT[DS_PROP_POS_BUMP]           = RE_PROP_BUMP_HIGH;
  propertyLUT[DS_PROP_POS_BUMP2]          = RE_PROP_BUMP_HIGH;
  propertyLUT[DS_PROP_DISP_MAP]           = RE_PROP_DISP_MAP;
  propertyLUT[DS_PROP_NEG_DISP]           = RE_PROP_DISP_LOW;
  propertyLUT[DS_PROP_NEG_DISP2]          = RE_PROP_DISP_LOW;
  propertyLUT[DS_PROP_POS_DISP]           = RE_PROP_DISP_HIGH;
  propertyLUT[DS_PROP_POS_DISP2]          = RE_PROP_DISP_HIGH;
  propertyLUT[DS_PROP_NORMAL_MAP]         = RE_PROP_NORMAL_MAP;
  propertyLUT[DS_PROP_AMBIENT_ON]         = RE_PROP_AMBIENT_ON;
  propertyLUT[DS_PROP_AMBIENT_COLOR]      = RE_PROP_AMBIENT_COLOR;
  propertyLUT[DS_PROP_AMBIENT_STRENGTH]   = RE_PROP_AMBIENT_VALUE;
  propertyLUT[DS_PROP_NORMAL_MAP]         = RE_PROP_NORMAL_MAP;

  // Set the conversion callback for each property
  propertyConverters[DS_PROP_DIFFUSE]       = &ReDSMaterialConverter::convertDiffuse;
  propertyConverters[DS_PROP_SPECULAR]      = &ReDSMaterialConverter::convertSpecular;
  propertyConverters[DS_PROP_SPECULAR2]     = &ReDSMaterialConverter::convertSpecular;
  propertyConverters[DS_PROP_IR_GLOSSY_CLR] = &ReDSMaterialConverter::convertSpecular;
  propertyConverters[DS_PROP_ALPHA]         = &ReDSMaterialConverter::convertAlpha;
  propertyConverters[DS_PROP_IR_ALPHA]      = &ReDSMaterialConverter::convertAlpha;
  propertyConverters[DS_PROP_BUMP_MAP]      = &ReDSMaterialConverter::convertBumpMap;
  propertyConverters[DS_PROP_NORMAL_MAP]    = &ReDSMaterialConverter::convertNormalMap;
  propertyConverters[DS_PROP_DISP_MAP]      = &ReDSMaterialConverter::convertDispMap;
  propertyConverters[DS_PROP_AMBIENT_COLOR] = &ReDSMaterialConverter::convertAmbient;
  propertyConverters[DS_PROP_U_TILE]        = &ReDSMaterialConverter::convertTiling;
  propertyConverters[DS_PROP_U_TILE2]       = &ReDSMaterialConverter::convertTiling;
  propertyConverters[DS_PROP_V_TILE]        = &ReDSMaterialConverter::convertTiling;
  propertyConverters[DS_PROP_V_TILE2]       = &ReDSMaterialConverter::convertTiling;
  propertyConverters[DS_PROP_U_OFFSET]      = &ReDSMaterialConverter::convertTiling;
  propertyConverters[DS_PROP_V_OFFSET]      = &ReDSMaterialConverter::convertTiling;
}

// Prototype
void updateMap( const QString mapName, const QString key, const QVariant& value );

QVariantMap createImageMapNode( const QString fName, 
                                const float gain,
                                const float uTile,
                                const float vTile,
                                const bool isNormalMap = false ) 
{
  QVariantMap im;
  im["type"]        = ReNodeConverter::ImageMap;
  im["fileName"]    = QVariant(fName);
  im["gain"]        = QVariant(gain);
  im["u offset"]    = QVariant(0.0);
  im["v offset"]    = QVariant(0.0);
  im["u tile"]      = QVariant(uTile);
  im["v tile"]      = QVariant(vTile);
  im["isNormalMap"] = isNormalMap;

  return(im);
}

//! The material properties that we return at the end of the conversion
QVariantMap matProps;

void initMatProps( const QString matID ) {
  // Remove the previous values  
  matProps.clear();
  // Create the map for the diffuse color
  QVariantMap colorMap;
  // The following is a default color used for several channels
  QList<QVariant> defaultColor;
  QColor tmp = RE_WHITE_COLOR;
  QList<QVariant> defaultWhite;
  defaultWhite << QVariant(tmp.redF()) << QVariant(tmp.greenF()) << QVariant(tmp.blueF());
  defaultColor << QVariant(1.0) << QVariant(1.0) << QVariant(1.0);

  // The diffuse channel has a color and a reference to a map
  colorMap["color"] = defaultWhite;
  colorMap["map"] = "";

  // Initialize the Diffuse channel
  matProps[RE_MAT_KEY_DIFFUSE] = colorMap;
  matProps[RE_MAT_KEY_DIFFUSE2] = colorMap;

  // Initialize the specular channel
  QVariantMap specMap;
  specMap["color"] = defaultColor;
  specMap["map"] = "";
  matProps[RE_MAT_KEY_SPECULAR] = specMap;

  // Specular 2
  QVariantMap spec2;
  QList<QVariant> blackColor;
  blackColor << QVariant(0.0) << QVariant(0.0) << QVariant(0.0);
  spec2["map"] = "";
  spec2["color"] = blackColor;
  spec2["roughness map"] = "";
  matProps[RE_MAT_KEY_SPECULAR2] = spec2;

  // Initialize the coat channel
  QVariantMap coatMap;
  coatMap["color"] = defaultColor;
  coatMap["map"] = "";
  matProps[RE_MAT_KEY_COAT] = coatMap;

  // Initialize the translucence channel
  QVariantMap translucenceMap;
  translucenceMap["color"] = defaultColor;
  translucenceMap["map"] = "";
  translucenceMap["on"] = false;
  matProps[RE_MAT_KEY_TRANSLUCENCE] = translucenceMap;

  // Initialize the bump map
  QVariantMap emptyMap;
  emptyMap["map"] = "";
  matProps[RE_MAT_KEY_BUMP] = emptyMap;
  matProps[RE_MAT_KEY_DISPLACEMENT] = emptyMap;

  matProps[RE_MAT_KEY_UROUGHNESS]  = 1.0;
  matProps[RE_MAT_KEY_VROUGHNESS]  = 1.0;
  matProps[RE_MAT_KEY_IS_SKIN]     = false;
  matProps[RE_MAT_KEY_LIGHT_GAIN]  = 0.0;
  matProps[RE_MAT_KEY_LIGHT_ALPHA] = false;

  // Ambient
  QVariantMap amb;
  amb["color"] = "";
  amb["map"]   = "";
  matProps[RE_MAT_KEY_AMBIENT] = amb;

  // Alpha channel
  QVariantMap alpha;
  alpha["map"] = "";
  alpha["strength"] = 1.0;  
  matProps[RE_MAT_KEY_ALPHA] = alpha;

  // The repository for all the maps/nodes
  matProps[RE_MAT_KEY_NODES_ROOT] = QVariantMap();
  matProps["name"] = matID;
  // By default the material is not defined.
  matProps["type"] = RE_MAT_TYPE_CODE_UNDEFINED;
  // Source, iRay
  matProps["source"] = "ir";
  // No preset
  matProps["preset"] = "";
}

QVariantMap* ReDSMaterialConverter::convertMaterial( DzMaterial* _dsMat ) {
  dsMat = _dsMat;
  // RE_LOG_INFO() << "DS Mat: " 
  //               << _dsMat->getName() 
  //               << " -- " 
  //               << _dsMat->metaObject()->className();
                 
  auto pli = dsMat->propertyListIterator();

  initMatProps(dsMat->getName());

  if ( dsMat->inherits("DzUberIrayMaterial") ) {
    isiRayShader = true;
    matProps["source"] = "ir";
  }
  else {
    isiRayShader = false;
    matProps["source"] = "3dl";
  }

  uTile = vTile = 1.0;
  uOffset = vOffset = 0.0;

  // Check if the material ha tags that indicate the type and properties
  auto tagProp = qobject_cast<DzStringProperty*>(
    dsMat->findProperty(DS_PROP_TAGS)
  );
  if (tagProp) {
    QString tags = tagProp->getValue();
    if (!tags.isEmpty()) {
      processMaterialTags(tags);
    }
  }

  while( pli.hasNext() ) {
    DzProperty* prop = pli.next();

    QString propName = prop->getName();
    // RE_LOG_INFO() << "Prop: " << propName.toStdString() 
    //               << " " << prop->metaObject()->className();
    if (propertyConverters.contains(propName)) {
      // Call the callback
      propertyConverters.value(propName)(this, prop);
    }
  }
  return &matProps;
}

//! Utility function used to update an entry in a QVariantMap
void updateMap( const QString mapName, const QString key, const QVariant& value ) {
  auto subMap = matProps[mapName].toMap();
  subMap[key] = value;
  matProps[mapName] = subMap;
}

QVariantList ReDSMaterialConverter::setColor( const QColor& clr )
{
  qreal r,g,b;
  clr.getRgbF(&r, &g, &b);
  QVariantList colorInfo;
  colorInfo << QVariant(r) << QVariant(g) << QVariant(b);
  return(colorInfo);
}

void ReDSMaterialConverter::processMaterialTags(const QString& tagStr) {
  auto tags = tagStr.split('-', QString::SkipEmptyParts);
  unsigned short int tagCount = tags.count();
  for (int i = 0; i < tagCount; i++) {
    tags[i] = tags[i].trimmed();
  }
  if (tags.contains("Glass")) {
    matProps["type"] = "GLS";
    if (tags.contains("Frosted")) {
      matProps["preset"] = "frosted";
    }
    else if(tags.contains("Solid")) {
      matProps["preset"] = "standard";
    }
    else if(tags.contains("Thin")) {
      matProps["preset"] = "architectural";
    }
  }
  else if (tags.contains("Metal")) {
    matProps["type"] = "MTL";
    if (tags.contains("Aluminum")) {
      matProps["preset"] = "aluminum";
    }
    else if (tags.contains("Steel")) {
      matProps["preset"] = "steel";
    }
  }
  else if (tags.contains("Fabric")) {
    if (tags.contains("Velvet")) {
      matProps["type"] = "VLV";
    }
    else {
      matProps["type"] = "CLO";
      if (tags.contains("Silk")) {
        matProps["preset"] = "silk";
      }
    }
  }
  else if (tags.contains("Carpaint")) {
    matProps["type"] = "GLO";
  }
  else if (tags.contains("Water")) {
    matProps["type"] = "WTR";
  }
}


void ReDSMaterialConverter::convertDiffuse(const DzProperty* prop) 
{
  // Convert from a generic property to the specific DzColorProperty,
  // which is what we expect to have in the Diffuse channel
  const DzColorProperty* diffProp = qobject_cast<const DzColorProperty*>(prop);
  if (!diffProp) {
    return;
  }
  // To change the nodes map we need to retrieve it, update it and then
  // replace it in the matProps dictionary.
  QVariantMap nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();
  auto* diffMap = diffProp->getMapValue();
  auto diffCol = diffProp->getColorValue();
  auto diffData = matProps[RE_MAT_KEY_DIFFUSE].toMap();
  float gain  = 1.0;

  // We can't have pure white, no matter what's in Studio's shader
  if (isPureWhite(diffCol) && !diffMap) {
    diffCol = RE_WHITE_COLOR;
  }
  diffData["color"].setValue(setColor(diffCol));
  if (diffMap) {
    QString nodeName = "DiffuseMap";
    // // Find the texture's gain
    // DzFloatProperty* gainProp = qobject_cast<DzFloatProperty*>(
    //   dsMat->findProperty(DS_PROP_DIFFUSE_STRENGTH)
    // );
    // if (gainProp) {
    //   gain = gainProp->getValue();
    // }
    gain = 1.0;
    nodes[nodeName] = QVariant(createImageMapNode(
      diffMap->getFilename(), gain, uTile, vTile)
    );
    diffData["map"] = QVariant(nodeName);
  }
  matProps[RE_MAT_KEY_DIFFUSE].setValue(diffData);
  matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);  
}

QVariantMap ReDSMaterialConverter::captureBumpMapData(const DzFloatProperty* bumpProp,
                                                      DzTexture* bumpMap ) 
{
  // Use a temporary map to store the values while we compute them
  QVariantMap bm = matProps[RE_MAT_KEY_BUMP].toMap();
  bm["hasMap"] = false;
  if (bumpMap) {
    bm["map"] = QVariant(createImageMapNode(
                  bumpMap->getFilename(), 1.0, uTile, vTile)
                );
    bm["hasMap"] = true;
    bm["strength"] = bumpProp->getValue();

    auto negBm = qobject_cast<DzFloatProperty*>(dsMat->findProperty(DS_PROP_NEG_BUMP));
    if (!negBm) {
      negBm = qobject_cast<DzFloatProperty*>(dsMat->findProperty(DS_PROP_NEG_BUMP2));      
    }
    auto posBm = qobject_cast<DzFloatProperty*>(dsMat->findProperty(DS_PROP_POS_BUMP));
    if (!posBm) {
      posBm = qobject_cast<DzFloatProperty*>(dsMat->findProperty(DS_PROP_POS_BUMP2));
    }
    float negBumpVal = -0.001,
          posBumpVal =  0.001;
    if (isiRayShader) {
      posBumpVal = RE_IRAY_DEFAULT_POS_BUMP;
      negBumpVal = RE_IRAY_DEFAULT_NEG_BUMP;
    }
    else {
      if (posBm) {
        posBumpVal = posBm->getValue() / bumpScale;
      }      
      if (negBm) {
        negBumpVal = negBm->getValue() / bumpScale;
      }     
    } 
    bm["pos"] = posBumpVal;
    bm["neg"] = negBumpVal;
  }
  return bm;
}

QVariantMap ReDSMaterialConverter::captureNormalMapData( DzTexture* normalMap ) {
  QVariantMap nm = matProps[RE_MAT_KEY_BUMP].toMap();
  nm["hasMap"] = false;
  
  if (normalMap) {
    auto nmStrengthProp = qobject_cast<DzFloatProperty*>(
                            dsMat->findProperty(DS_PROP_NORMAL_MAP_STRENGHT)
                          );
    nm["hasMap"] = true;

    float strength = 1.0;
    if (nmStrengthProp) {
      strength = nmStrengthProp->getValue();
    }
    nm["map"] = QVariant(
      createImageMapNode(
        normalMap->getFilename(), 
        1.0, 
        uTile, 
        vTile,
        true
      )
    );
    nm["strength"] = strength;
  }

  return nm;
}

void ReDSMaterialConverter::mixBumpMapData( QVariantMap& bmData, QVariantMap& nmData ) {
  // auto bmTex = bmData["map"].toMap();
  // auto nmTex = nmData["map"].toMap();

  auto nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();

  bool hasBump = bmData["hasMap"].toBool();
  bool hasNormal = nmData["hasMap"].toBool();
  // Bump Map only
  // bmData["map"]: {"type": ReNodeConverter::ImageMap,
  //                 "fileName":    QVariant(fName),
  //                 "gain":        QVariant(gain),
  //                 "u offset":    QVariant(0.0),
  //                 "v offset":    QVariant(0.0),
  //                 "u tile":      QVariant(uTile),
  //                 "v tile":      QVariant(vTile),
  //                 "isNormalMap": isNormalMap},
  // bmData["hasMap"]: true,
  // bmData["strength"]: QVariant(strength),
  // bmData["pos"]: QVariant(pos),
  // bmData["neg"]: QVariant(neg)
  //

  if (hasBump && !hasNormal) {
    // Add the map data to the nodes
    nodes["bump"] = bmData["map"];
    // Replace the map with a reference to it
    bmData["map"] = "bump";
    // Store the bump map data in the material properties
    matProps["bump"] = bmData;
  }
  // Normal map only
  else if(hasNormal && !hasBump) {
    nodes["bump"] = nmData["map"];
    nmData["map"] = "bump";
    matProps["bump"] = nmData;
  }
  // Bump map and normal map together. Make a Math texture.
  else if (hasBump && hasNormal) {
    QVariantMap mathTex;
    mathTex["type"] = ReNodeConverter::Math;
    // Save the bump map into the nodes
    nodes["bump"] = bmData["map"];
    // Save the normal map into the nodes
    nodes["nm"] = nmData["map"];
    // The first texture is the bump map
    QVariantMap val1;
    val1["map"] = "bump";
    // This value must be scaled so for the mix to work. Otherwise the bump map
    // will overpower the normal map.
    val1["value"] = 0.0008;
    mathTex["value 1"] = val1;
    // The second texture is the normal map
    QVariantMap val2;
    val2["map"] = "nm";
    val2["value"] = 1.0;
    mathTex["value 2"] = val2;
    // We add the two textures together
    mathTex["function"] = "a";
    // Add the Mix texture to the node list
    nodes["bmMixer"] = mathTex;
    // Set the Mix texture as the material's bump map
    bmData["map"] = "bmMixer";
    // Correction for bump map strength. The value in Studio is too high
    // for Lux
    bmData["strength"] = bmData["strength"].toFloat() * RE_BUMP_STRENGTH_CORRECTION;
    matProps["bump"] = bmData;
  }
  matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);
};

void ReDSMaterialConverter::convertBumpMap( const DzProperty* prop) 
{
  auto bumpProp = qobject_cast<const DzFloatProperty*>(prop);
  if (!bumpProp) {
    return;
  }
  auto bumpMap = bumpProp->getMapValue();
  auto nmProp = dsMat->findProperty(DS_PROP_NORMAL_MAP);

  DzTexture* normalMap = NULL;
  // Normal Map properties don't have consistent type. If they come from
  // 3Delight they are of type DzFloatProperty. If they come from iRay
  // they are of type DzImageProperty
  if (nmProp != NULL) {
    if (nmProp->inherits("DzImageProperty")) {
      normalMap = static_cast<const DzImageProperty*>(nmProp)->getValue();
    }
    else if (nmProp->inherits("DzFloatProperty")) {
      normalMap = static_cast<const DzFloatProperty*>(nmProp)->getMapValue();
    }
  }
  if (!bumpMap && !normalMap) {
    return;
  }

  auto bmData = captureBumpMapData(bumpProp, bumpMap);
  auto nmData = captureNormalMapData(normalMap);

  mixBumpMapData(bmData, nmData);
}

void ReDSMaterialConverter::convertNormalMap( const DzProperty* prop ) {
  auto nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();
  // Check if there is a bump map too. If so then we need to exit
  // because the mixing of bump and normal maps is done by the bump map
  // conversion methods.
  auto bmProp = dsMat->findProperty(DS_PROP_BUMP_MAP);
  if (bmProp != NULL) {
    DzTexture* bmTex;
    if (bmProp->inherits("DzImageProperty")) {
      bmTex = static_cast<const DzImageProperty*>(bmProp)->getValue();
    }
    else if (bmProp->inherits("DzFloatProperty")) {
      bmTex = static_cast<const DzFloatProperty*>(bmProp)->getMapValue();
    }
    if (bmTex != NULL) {
      return;
    } 
  }

  DzTexture* normalMap = NULL;
  // Normal Map properties don't have consistent type. If they come from
  // 3Delight they are of type DzFloatProperty. If they come from iRay
  // they are of type DzImageProperty
  if (prop->inherits("DzImageProperty")) {
    normalMap = static_cast<const DzImageProperty*>(prop)->getValue();
  }
  else if (prop->inherits("DzFloatProperty")) {
    normalMap = static_cast<const DzFloatProperty*>(prop)->getMapValue();
  }
  else {
    // should never happen, but just in case
    return;
  }
  if (normalMap == NULL) {
    return;
  }
  auto nmData = captureNormalMapData(normalMap);
  // Store the Normal Map into the nodes. The map info has the flag 
  // that this is a normal map
  nodes["bump"] = nmData["map"];
  // Replace the map with a reference to it, in the temp data that we 
  // have collected
  nmData["map"] = "bump";
  // Store the bump/normal map data in the material properties.
  matProps["bump"] = nmData;
  matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);  
}

void ReDSMaterialConverter::convertDispMap( const DzProperty* prop) 
{
  auto* dispProp = qobject_cast<const DzFloatProperty*>(prop);
  if (!dispProp) {
    return;
  }
  QVariantMap nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();
  auto dispMap = dispProp->getMapValue();
  if (dispMap) {
    QString nodeName = RE_MAT_KEY_DISPLACEMENT;
    nodes[nodeName] = QVariant(createImageMapNode(
      dispMap->getFilename(), 1.0, uTile, vTile)
    );
    updateMap(RE_MAT_KEY_DISPLACEMENT, "map", nodeName);
    updateMap(RE_MAT_KEY_DISPLACEMENT, "strength", dispProp->getValue());
    auto negDisp = qobject_cast<DzFloatProperty*>(
      dsMat->findProperty(DS_PROP_NEG_DISP)
    );
    if (!negDisp) {
      negDisp = qobject_cast<DzFloatProperty*>(
        dsMat->findProperty(DS_PROP_NEG_DISP2)
      );      
    }
    auto posDisp = qobject_cast<DzFloatProperty*>(
      dsMat->findProperty(DS_PROP_POS_DISP)
    );
    if (!posDisp) {
      posDisp = qobject_cast<DzFloatProperty*>(
        dsMat->findProperty(DS_PROP_POS_DISP2)
      );
    }
    float negDispVal = -0.001,
          posDispVal =  0.001;
    if (posDisp) {
      posDispVal = posDisp->getValue() / displacementScale;
    }      
    if (negDisp) {
      negDispVal = negDisp->getValue() / displacementScale;
    }      
    updateMap(RE_MAT_KEY_DISPLACEMENT, "pos", posDispVal);
    updateMap(RE_MAT_KEY_DISPLACEMENT, "neg", negDispVal);
    matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);
  }
}

void ReDSMaterialConverter::convert3DLSpecular( const DzProperty* prop ) {
  auto specProp = qobject_cast<const DzColorProperty*>(prop);
  if (!specProp) {
    return;
  }
  bool isSpecular2 = isProperty(prop->getName(), RE_PROP_SPECULAR2);
  if (isSpecular2) {
    auto spec2On = qobject_cast<DzBoolProperty*>(
                     dsMat->findProperty(DS_PROP_SPECULAR2_ON)
                   );
    if ( !spec2On->getBoolValue() ) {
      return;
    }
  }

  QVariantMap nodes = matProps["nodes"].toMap();
  DzTexture* specMap = specProp->getMapValue();
  QColor specCol = specProp->getColorValue();
  QString specKey = isSpecular2 ? RE_MAT_KEY_SPECULAR2 : RE_MAT_KEY_SPECULAR;
  QVariantMap specData = matProps[specKey].toMap();
  float gain  = 1.0;
  float uRoughness = 0.1,
        vRoughness = 0.1;

  // Find the specular strength
  DzFloatProperty* gainProp = qobject_cast<DzFloatProperty*>(
    dsMat->findProperty(isSpecular2 ? DS_PROP_SPECULAR2_STRENGTH : DS_PROP_SPECULAR_STRENGTH)
  );
  if (gainProp) {
    gain = gainProp->getValue();
  }
  // Dim the specular color if the specular strength is less than 100%
  if (gain < 1.0f) {
    dimColor(specCol, gain);
  }
  // Store the color value
  specData["color"].setValue(setColor(specCol));

  QString nodeName = "SpecMap";
  // Glossiness value
  DzFloatProperty* glossinessProp = qobject_cast<DzFloatProperty*>(
    dsMat->findProperty(DS_PROP_GLOSSINESS)
  );
  if (!glossinessProp) {
    return;
  }
  // Reality's intermediate data requires this value to be the roughness
  // instead of the glossiness of the material.
  uRoughness = vRoughness = 1.0 - glossinessProp->getValue();

  matProps[RE_MAT_KEY_UROUGHNESS] = QVariant(uRoughness);
  matProps[RE_MAT_KEY_VROUGHNESS] = QVariant(vRoughness);

  // Look if there is a glossiness strength map
  auto glossinessMap = glossinessProp->getMapValue();
  if (glossinessMap) {
    nodes["roughness map"] = QVariant(createImageMapNode(
      glossinessMap->getFilename(), gain, uTile, vTile)
    );
    matProps["roughness map"] = QVariant( QString("roughness map") );
  }

  if (specMap) {
    // Add the specular node
    nodes[nodeName] = QVariant(createImageMapNode(
      specMap->getFilename(), gain, uTile, vTile)
    );
    specData["map"] = QVariant(nodeName);
  }
  matProps["nodes"].setValue(nodes);
  matProps[specKey].setValue(specData);
}

void ReDSMaterialConverter::convertIRaySpecular(const DzProperty* prop) {
  auto specProp = qobject_cast<const DzColorProperty*>(prop);
  if (!specProp) {
    return;
  }
  auto propName = specProp->getName();

  QVariantMap nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();
  DzTexture* specMap = specProp->getMapValue();
  QColor specCol = specProp->getColorValue();
  QVariantMap specData = matProps[RE_MAT_KEY_SPECULAR].toMap();
  float specMapGain  = 1.0;
  float uRoughness = RE_IRAY_BASE_ROUGHNESS,
        vRoughness = RE_IRAY_BASE_ROUGHNESS;

  auto irSpecMapProp = qobject_cast<DzFloatProperty*>(
                         dsMat->findProperty(DS_PROP_IR_GLOSSY_LAYERED_WGT)
                       );
  if (irSpecMapProp) {
    specMap = irSpecMapProp->getMapValue();
    specMapGain = RE_IRAY_SPECMAP_ADJUSTED_GAIN;
  }
  // Store the color value
  specData["color"].setValue(setColor(specCol));

  QString nodeName = "SpecMap";
  // Glossiness value
  DzFloatProperty* glossinessProp = qobject_cast<DzFloatProperty*>(
    dsMat->findProperty(DS_PROP_IR_ROUGHNESS)
  ); 
  if (glossinessProp) {
    uRoughness = vRoughness = (RE_IRAY_BASE_ROUGHNESS - glossinessProp->getValue());
  }
  // Check the value of reflectivity. The value of RE_IRAY_BASE_ROUGHNESS is based
  // on a reflectivity of 0. According to the iRay documentation at DAZ, this
  // property defaults to 0.5, which would increase the shine of the material from
  // the RE_IRAY_BASE_ROUGHNESS point. The conversion to Reality's values is based
  // on empirical tests and it works by subtracting a value from u/v roughness
  // that is proportional to the value of the reflectivity property. In this
  // case we ignore any possible map attached to this channel.
  DzFloatProperty* reflectivityProp = qobject_cast<DzFloatProperty*>(
    dsMat->findProperty(DS_PROP_IR_GLOSSY_GLOSSY_REFLECTIVITY)
  ); 
  if (reflectivityProp) {
    uRoughness *= (1.0 - reflectivityProp->getValue());
    vRoughness = uRoughness;
  }

  matProps[RE_MAT_KEY_UROUGHNESS] = QVariant(uRoughness);
  matProps[RE_MAT_KEY_VROUGHNESS] = QVariant(vRoughness);

  // Look if there is a glossiness strength map
  auto glossinessMap = glossinessProp->getMapValue();
  if (glossinessMap) {
    nodes["roughness map"] = QVariant(createImageMapNode(
      glossinessMap->getFilename(), 1.0, uTile, vTile)
    );
    matProps["roughness map"] = QVariant( QString("roughness map") );
  }

  if (specMap) {
    // Add the specular node
    nodes[nodeName] = QVariant(createImageMapNode(
      specMap->getFilename(), specMapGain, uTile, vTile)
    );
    specData["map"] = QVariant(nodeName);
  }

  // Look for translucency data
  auto transProp = qobject_cast<DzColorProperty*>(
    dsMat->findProperty(DS_PROP_IR_TRANSLUCENCY)
  ); 
  if (transProp) {
    auto transMap = transProp->getMapValue();
    auto transClr = transProp->getColorValue();
    QVariantMap transData;

    if (transMap) {
      nodes[RE_MAT_KEY_TRANSLUCENCE] = QVariant(createImageMapNode(
        transMap->getFilename(), 1.0, uTile, vTile)
      );
      transData["map"] = QVariant(RE_MAT_KEY_TRANSLUCENCE);
    }
    transData["color"] = setColor(transClr);
    transData["on"] = true;
    matProps[RE_MAT_KEY_TRANSLUCENCE] = transData;
  }

  // Look for metallic flake color, which we assign to the coat
  auto metFlakeClrProp = qobject_cast<DzColorProperty*>(
    dsMat->findProperty(DS_PROP_IR_METALLIC_CLR)
  ); 
  if (metFlakeClrProp) {
    auto mfcMap = metFlakeClrProp->getMapValue();
    auto mfcClr = metFlakeClrProp->getColorValue();
    QVariantMap coatData;

    if (mfcMap ) {
      nodes[RE_MAT_KEY_COAT] = QVariant(createImageMapNode(
        mfcMap->getFilename(), 1.0, uTile, vTile)
      );
      coatData["map"] = QVariant(RE_MAT_KEY_COAT);
    }
    coatData["color"] = setColor(mfcClr);
    matProps[RE_MAT_KEY_COAT] = coatData;
  }

  matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);
  matProps[RE_MAT_KEY_SPECULAR].setValue(specData);
}

void ReDSMaterialConverter::convertSpecular(const DzProperty* prop) 
{
  if (isiRayShader) {
    convertIRaySpecular(prop);
  }
  else {
    convert3DLSpecular(prop);
  }
}

void ReDSMaterialConverter::convertAmbient(const DzProperty* prop) 
{
  auto ambProp = qobject_cast<const DzColorProperty*>(prop);
  if (!ambProp) {
    return;
  }
  auto ambColor = ambProp->getColorValue();
  if (!isColorEmitting(ambColor)) {
    return;
  }
  QVariantMap nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();
  // Some materials have the "Ambient ON" property, some don't.
  // If it's present and it's turned off then exit
  auto ambPropSwitch = qobject_cast<DzBoolProperty*>(
    dsMat->findProperty(DS_PROP_AMBIENT_ON)
  );
  if ( ambPropSwitch && !ambPropSwitch->getBoolValue() ) {
    return;
  }

  auto ambMap = ambProp->getMapValue();
  // Find the ambient strength and set the light gain to it
  auto ambGain = qobject_cast<DzFloatProperty*>(
    dsMat->findProperty(DS_PROP_AMBIENT_STRENGTH)
  );
  float ambGainValue = 0.0;
  if (ambGain) {
    ambGainValue = ambGain->getValue();
  }
  matProps[RE_MAT_KEY_LIGHT_GAIN] = QVariant(ambGainValue);

  QString nodeName = RE_MAT_KEY_AMBIENT;
  updateMap(RE_MAT_KEY_AMBIENT, "color", setColor(ambColor));
  if (ambMap) {
    nodes[nodeName] = QVariant(createImageMapNode(
      ambMap->getFilename(), 1.0, uTile, vTile)
    );
    updateMap(RE_MAT_KEY_AMBIENT, "map", nodeName);
    matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);    
  }
  matProps[RE_MAT_KEY_LIGHT_ALPHA] = QVariant(false);
}

void ReDSMaterialConverter::convertAlpha(const DzProperty* prop) 
{
  QVariantMap nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();
  auto* alphaProp = qobject_cast<const DzFloatProperty*>(prop);
  if (!alphaProp) {
    return;
  }
  updateMap(RE_MAT_KEY_ALPHA,"strength", alphaProp->getValue());
  DzTexture* alphaMap = alphaProp->getMapValue();
  if (alphaMap) {
    QString nodeName = "alphaMap";
    nodes[nodeName] = QVariant(createImageMapNode(
      alphaMap->getFilename(), 1.0, uTile, vTile)
    );
    matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);
    updateMap(RE_MAT_KEY_ALPHA, "map", nodeName);
  }  
}

void ReDSMaterialConverter::convertTiling(const DzProperty* prop)
{
  // This is the function that is executed in the loop. It 
  // updates the map's "u tile" or "v tile" elements with
  // a bound value. We use boost::bind to bind the value to the actual
  // lambda function.
  boost::function<void ( QVariantMap& m  )> tileUpdater;
  // Update all the image maps in the nodes to use the new values
  QVariantMap nodes = matProps[RE_MAT_KEY_NODES_ROOT].toMap();
  QString propName = prop->getName();

  if ( isProperty(propName, RE_PROP_H_TILE) ) {
    // Store the value for future uses in case image map nodes are 
    // found *after* the tiling properties are processed
    uTile = qobject_cast<const DzFloatProperty*>(prop)->getValue();
    // Prepare the lamdba function and bind the uTile variable
    // to the first parameter
    tileUpdater = boost::bind<void>([]( float v, QVariantMap& m ) {
                                      m["u tile"] = v;
                                    }, uTile, boost::placeholders::_1);
  }
  else if (isProperty(propName, RE_PROP_V_TILE)) {
    // Store the value for future uses in case image map nodes are 
    // found *after* the tiling properties are processed
    vTile = qobject_cast<const DzFloatProperty*>(prop)->getValue();
    // Prepare the lamdba function and bind the uTile variable
    // to the first parameter
    tileUpdater = boost::bind<void>([]( float v, QVariantMap& m ) {
                                      m["v tile"] = v;
                                    }, vTile, boost::placeholders::_1);
  }
  else if (isProperty(propName, RE_PROP_H_OFFSET)) {
    // Store the value for future uses in case image map nodes are 
    // found *after* the tiling properties are processed
    uOffset = qobject_cast<const DzFloatProperty*>(prop)->getValue();
    // Prepare the lamdba function and bind the uTile variable
    // to the first parameter
    tileUpdater = boost::bind<void>([]( float v, QVariantMap& m ) {
                                      m["u offset"] = v;
                                    }, uOffset, boost::placeholders::_1);
  }
  else if (isProperty(propName, RE_PROP_V_OFFSET)) {
    // Store the value for future uses in case image map nodes are 
    // found *after* the tiling properties are processed
    vOffset = qobject_cast<const DzFloatProperty*>(prop)->getValue();
    // Prepare the lamdba function and bind the uTile variable
    // to the first parameter
    tileUpdater = boost::bind<void>([]( float v, QVariantMap& m ) {
                                      m["v offset"] = v;
                                    }, vOffset, boost::placeholders::_1);
  }
  // Scan all the nodes and find the image maps. 
  QMapIterator<QString, QVariant> i(nodes);
  while( i.hasNext() ) {
    i.next();
    QVariantMap n = i.value().toMap();
    if (n["type"] == ReNodeConverter::ImageMap) {
      // Update the tile parameter
      tileUpdater(n);
      // Replace the value back in the nodes map
      nodes[i.key()] = n;
    }
  }
  // Replace the value back in the material definition map
  matProps[RE_MAT_KEY_NODES_ROOT].setValue(nodes);
}

} // namespace
