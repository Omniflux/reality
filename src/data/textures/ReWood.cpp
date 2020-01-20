/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReWood.h"
#include "ReTools.h"

namespace Reality {

// initialization of static variables
QString ReWood::lutPatternsToStrings[ReWood::RING_NOISE+1] = {
  QString(QObject::tr("Rings")),
  QString(QObject::tr("Bands")),
  QString(QObject::tr("Wavy Bands")),
  QString(QObject::tr("Wavy Rings")),
};

QString ReWood::lutPatternsToNames[ReWood::RING_NOISE+1] = {
  QString("rings"),
  QString("bands"),
  QString("bandnoise"),
  QString("ringnoise"),
};

// Constructor: ReWood
ReWood::ReWood(const QString name, ReTextureContainer* parentMat ) : 
  ReProceduralNoise(name, parentMat),
  turbulence(5),
  veinWave(SIN),
  woodPattern(RING_NOISE),
  softNoise(true)
{
  type = TexWood;
  textureDataType = numeric;
  noiseBasis = BLENDER_ORIGINAL;
};

// Conversion ctor
ReWood::ReWood( const ReTexturePtr srcTex ) :
  ReProceduralNoise(srcTex),
  turbulence(5),
  veinWave(SIN),
  woodPattern(RING_NOISE),
  softNoise(true)
{
  type = TexWood;
  textureDataType = numeric;
  noiseBasis = BLENDER_ORIGINAL;

  if ( srcTex->getType() == TexWood ) {
    ReWoodPtr t2 = srcTex.staticCast<ReWood>();
    noiseSize   = t2->getNoiseSize();
    brightness  = t2->getBrightness();
    contrast    = t2->getContrast();
    noiseBasis  = t2->getNoiseBasis();
    turbulence  = t2->turbulence;
    veinWave    = t2->veinWave;
    softNoise   = t2->softNoise;
    woodPattern = t2->woodPattern;
  }
}

float ReWood::getTurbulence() const {
  return turbulence;
};

void ReWood::setTurbulence( float newVal ) {
  turbulence = newVal;
};

ReWood::VeinWave ReWood::getVeinWave() const {
  return veinWave;
};

bool ReWood::hasSoftNoise() const {
  return softNoise;
};

void ReWood::setSoftNoise( bool newVal ) {
  softNoise = newVal;
};

const QString ReWood::getVeinWaveAsString() const {
  switch( veinWave ) {
    case SIN:
      return "sin";
    case SAWTOOTH:
      return "saw";
    case TRIANGLE:
      return "tri";
  }
  return "sin";
};


void ReWood::setVeinWave( VeinWave newVal ) {
  veinWave = newVal;
};

ReWood::WoodPattern ReWood::getWoodPattern() const {
  return woodPattern;
};

const QString ReWood::getWoodPatternAsString() const {
  switch( woodPattern ) {
    case RINGS:
      return "rings";
    case BANDS:
      return "bands";
    case RING_NOISE:
      return "ringnoise";
    case BAND_NOISE:
      return "bandnoise";
  }
  return "rings";
};


void ReWood::setWoodPattern( WoodPattern newVal ) {
  woodPattern = newVal;
};


void ReWood::setNamedValue( const QString& name, const QVariant& value ) {
  if ( name == "turbulence" ) {
    turbulence = value.toFloat();
  }
  else if ( name == "veinWave" ) {
    veinWave = static_cast<VeinWave>(value.toInt());
  }
  else if ( name == "softNoise" ) {
    softNoise = value.toBool();
  }
  else if ( name == "woodPattern" ) {
    woodPattern = static_cast<WoodPattern>(value.toInt());
  }
  else {
    ReProceduralNoise::setNamedValue(name, value);
  }
};

const QVariant ReWood::getNamedValue( const QString& name ) {
  if ( name == "turbulence" ) {
    return turbulence;
  }
  else if ( name == "veinWave" ) {
    return veinWave;
  }
  else if ( name == "softNoise" ) {
    return softNoise;
  }
  else if ( name == "woodPattern" ) {
    return woodPattern;
  }
  else {
    return ReProceduralNoise::getNamedValue(name);
  }
};

void ReWood::serialize( QDataStream& dataStream ) {
  ReProceduralNoise::serialize(dataStream);
  dataStream << turbulence
             << (quint16) veinWave
             << (quint16) woodPattern
             << softNoise;
};

void ReWood::deserialize( QDataStream& dataStream ) {
  ReProceduralNoise::deserialize(dataStream);
  quint16 veinWaveInt,
          woodPatternInt;
  dataStream >> turbulence
             >> veinWaveInt
             >> woodPatternInt
             >> softNoise;
  veinWave = static_cast<VeinWave>(veinWaveInt);
  woodPattern = static_cast<WoodPattern>(woodPatternInt);
};


} // namespace
