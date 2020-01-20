/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReWater.h"
#include "ReTools.h"
#include "textures/ReProceduralNoise.h"

#define WATER_DM_STRENGTH_CLAMP 10.0f
#define WATER_NOISE_SIZE        0.25
#define WATER_DISTORTION_AMOUNT 1.6
#define WATER_BRIGHTNESS        1.0
#define WATER_CONTRAST          0.5
#define WATER_DM_NEGATIVE       -0.65f
#define WATER_DM_POSITIVE       0.5f
#define WATER_SUBDIVISION       2.0f 
#define WATER_RIPPLE_AMOUNT     0.5f 
#define WATER_IOR               1.33157f 
#define WATER_CLARITY_AT_DEPTH  0.1f

namespace Reality {

QVariantMap ReWater::ripplePresets[WM_NUM_RIPPLE_PRESETS];
bool ReWater::presetsInitialized = false;
QString ReWater::ripplePresetsNames[WM_NUM_RIPPLE_PRESETS] = {
  QString(QObject::tr("Classic")),
  QString(QObject::tr("Frothy")),
  QString(QObject::tr("Bubbly")),
  QString(QObject::tr("Classic 2")),
  QString(QObject::tr("Light")),
  QString(QObject::tr("Frosty"))
};

ReWater::ReWater( const QString name, const ReGeometryObject* parent ) : 
  DisplaceableMaterial(name, parent),
  ripples(WATER_RIPPLE_AMOUNT),
  ripplePreset(0)
{
  type = originalType = MatWater;

  makeNewTexture("Dm", TexDistortedNoise);
  ReTexturePtr dm = channels["Dm"];
  dm->setNamedValue("distortionAmount", WATER_DISTORTION_AMOUNT);
  dm->setNamedValue("noiseSize",        WATER_NOISE_SIZE);
  dm->setNamedValue("noiseBasis",       ReProceduralNoise::BLENDER_ORIGINAL);
  dm->setNamedValue("distortionType",   ReProceduralNoise::BLENDER_ORIGINAL);
  dm->setNamedValue("brightness",       WATER_BRIGHTNESS);
  dm->setNamedValue("contrast",         WATER_CONTRAST);
  dm->setNamedValue("mapping",          ReTexture3D::GLOBAL);
  // Better to disable this for water
  setKeepSharpEdges(false);
  setDmNegative(WATER_DM_NEGATIVE);
  setDmPositive(WATER_DM_POSITIVE);
  setDmStrength(ripples/WATER_DM_STRENGTH_CLAMP);
  setSubdivisions(WATER_SUBDIVISION);

  // Initialize the presets only once
  if (!presetsInitialized) {
    ripplePresets[0]["noiseBasis"]     = ReProceduralNoise::BLENDER_ORIGINAL;
    ripplePresets[0]["distortionType"] = ReProceduralNoise::BLENDER_ORIGINAL;
    
    ripplePresets[1]["noiseBasis"]     = ReProceduralNoise::VORONOI_F4;
    ripplePresets[1]["distortionType"] = ReProceduralNoise::BLENDER_ORIGINAL;
    
    ripplePresets[2]["noiseBasis"]     = ReProceduralNoise::VORONOI_F2_F1;
    ripplePresets[2]["distortionType"] = ReProceduralNoise::BLENDER_ORIGINAL;
    
    ripplePresets[3]["noiseBasis"]     = ReProceduralNoise::IMPROVED_PERLIN;
    ripplePresets[3]["distortionType"] = ReProceduralNoise::BLENDER_ORIGINAL;
    
    ripplePresets[4]["noiseBasis"]     = ReProceduralNoise::IMPROVED_PERLIN;
    ripplePresets[4]["distortionType"] = ReProceduralNoise::VORONOI_F3;
    
    ripplePresets[5]["noiseBasis"]     = ReProceduralNoise::VORONOI_CRACKLE;
    ripplePresets[5]["distortionType"] = ReProceduralNoise::VORONOI_F3;

    presetsInitialized = true;
  }
  createVolume();
}

void ReWater::createVolume() {
  QString parentName = getParent()->getInternalName();
  QString volumeName = QString("%1:water").arg(parentName);
  if (!RealitySceneData->volumeExists(volumeName)) {
    ReVolumePtr vol = ReVolumePtr( new ReVolume() );
    vol->setType(ReVolume::clear);
    vol->setName(volumeName);
    vol->setColor(QColor(250, 250, 250));
    // Water at room temperature
    vol->setIOR(WATER_IOR);
    vol->setClarityAtDepth(WATER_CLARITY_AT_DEPTH);
    vol->setEditable(false);
    RealitySceneData->saveVolume(vol);
  }
  RealitySceneData->linkVolumeToMaterial(volumeName, parentName, name);
  RealitySceneData->linkVolumeToMaterial(RE_AIR_VOLUME, parentName, name);

  setInnerVolume(volumeName);
  setOuterVolume(RE_AIR_VOLUME);
}

QColor ReWater::getKt() const {
  ReVolumePtr vol = getInnerVolume();
  if (vol.isNull()) {
    return QColor();
  }
  return vol->getColor();
};

void ReWater::setKt( QColor newVal ) {
  ReVolumePtr vol = getInnerVolume();
  if (vol.isNull()) {
    return;
  }
  vol->setColor(newVal);
};

float ReWater::getClarityAtDepth() const {
  ReVolumePtr vol = getInnerVolume();
  if (vol.isNull()) {
    return WATER_CLARITY_AT_DEPTH;
  }
  return vol->getClarityAtDepth();
};

void ReWater::setClarityAtDepth( float newVal ) {
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      vol->setClarityAtDepth(newVal);
    }
  }
};

float ReWater::getIOR() const {
  // Connects to the volume
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      return vol->getIOR();
    }
  }
  return WATER_IOR;
};

void ReWater::setIOR( float newVal ) {
  // Synchronizes the connected volume
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      vol->setIOR(newVal);
    }
  }
};

void ReWater::setRipples( float newVal ) {
  ripples = newVal;
  setDmStrength(ripples/WATER_DM_STRENGTH_CLAMP);
};

void ReWater::fromMaterial( const ReMaterial* baseMat ) {
  // When converting from another material we don't want to carry over
  // the displacement information from there. The water rippling effect is 
  // caused by very specific parameters designed for the Water material
  // and we need to retain those.
  
  // See if the source material uses the Diffuse channel, if so then copy that
  // texture to the transmission channel of water
  auto tex = baseMat->getChannel("Kd");
  if (!tex.isNull() && tex->getType() == TexConstant) {
    auto vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      vol->setColor(tex.staticCast<ReConstant>()->getColor());
    }
  }
}

void ReWater::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "ripples") {
    setRipples(value.toFloat());
  }
  if (vname == "ripplePreset") {
    ripplePreset = value.toInt();
  }
  else if ( vname == "clarityAtDepth" ) {
    setClarityAtDepth(value.toFloat());
  }
  else if ( vname == "IOR" ) {
    setIOR(value.toFloat());
  }
  else if ( vname == "Kt" ) {
    setKt(value.value<QColor>());
  }
  else {
    DisplaceableMaterial::setNamedValue(vname, value);
  }
};

const QVariant ReWater::getNamedValue( const QString& vname ) const {
  if (vname == "ripples") {
    return ripples;
  }
  if (vname == "ripplePreset") {
    return ripplePreset;
  }
  else if ( vname == "clarityAtDepth" ) {
    return getClarityAtDepth();
  }
  else if ( vname == "IOR" ) {    
    return getIOR();
  }
  else if ( vname == "Kt" ) {    
    return getKt();
  }
  return DisplaceableMaterial::getNamedValue(vname);
};

void ReWater::serialize( QDataStream& dataStream ) const { 
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << ripples << ripplePreset;
  DisplaceableMaterial::serialize(dataStream);      
};


void ReWater::deserialize( QDataStream& dataStream ) {
  ReMaterial::deserialize( dataStream );
  dataStream >> ripples >> ripplePreset;
  DisplaceableMaterial::deserialize(dataStream);      
}


} // namespace
