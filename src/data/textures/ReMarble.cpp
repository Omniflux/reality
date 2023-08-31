/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReMarble.h"


namespace Reality {

// Constructor: ReMarble
ReMarble::ReMarble(const QString name, ReTextureContainer* parentMat ) : 
  ReProceduralNoise(name, parentMat),
  depth(3),
  turbulence(5),
  veinQuality(SOFT),
  veinWave(SIN),
  softNoise(true)
{
  textureDataType = numeric;
  type = TexMarble; 
};

// Conversion ctor
ReMarble::ReMarble( const ReTexturePtr srcTex ) :
  ReProceduralNoise(srcTex),
  depth(3),
  turbulence(5),
  veinQuality(SOFT),
  veinWave(SIN),
  softNoise(true)
{
  textureDataType = numeric;
  type = TexMarble;

  if ( srcTex->getType() == TexMarble ) {
    ReMarblePtr t2 = srcTex.staticCast<ReMarble>();
    depth       = t2->depth;
    turbulence  = t2->turbulence;
    veinQuality = t2->veinQuality;
    veinWave    = t2->veinWave;
    softNoise   = t2->softNoise;
  }
}

float ReMarble::getDepth() const {
  return depth;
};

void ReMarble::setDepth( float newVal ) {
  depth = newVal;
};

float ReMarble::getTurbulence() const {
  return turbulence;
};

void ReMarble::setTurbulence( float newVal ) {
  turbulence = newVal;
};

ReMarble::VeinQuality ReMarble::getVeinQuality() const {
  return veinQuality;
};

const QString ReMarble::getVeinQualityAsString() const {
  switch( veinQuality ) {
    case SOFT:
      return "soft";
    case SHARP:
      return "sharp";
    case SHARPER:
      return "sharper";
  }
  return "soft";
}


void ReMarble::setVeinQuality( VeinQuality newVal ) {
  veinQuality = newVal;
};

ReMarble::VeinWave ReMarble::getVeinWave() const {
  return veinWave;
};

bool ReMarble::hasSoftNoise() const {
  return softNoise;
};

void ReMarble::setSoftNoise( bool newVal ) {
  softNoise = newVal;
};

const QString ReMarble::getVeinWaveAsString() const {
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


void ReMarble::setVeinWave( VeinWave newVal ) {
  veinWave = newVal;
};


void ReMarble::setNamedValue( const QString& name, const QVariant& value ) {
  if ( name == "depth" ) {
    depth = value.toFloat();
  }
  else if ( name == "turbulence" ) {
    turbulence = value.toFloat();
  }
  else if ( name == "veinQuality" ) {
    veinQuality = static_cast<VeinQuality>(value.toInt());
  }
  else if ( name == "veinWave" ) {
    veinWave = static_cast<VeinWave>(value.toInt());
  }
  else if ( name == "softNoise" ) {
    softNoise = value.toBool();
  }
  else {
    ReProceduralNoise::setNamedValue(name, value);
  }
};

const QVariant ReMarble::getNamedValue( const QString& name ) {
  if ( name == "depth" ) {
    return depth;
  }
  else if ( name == "turbulence" ) {
    return turbulence;
  }
  else if ( name == "veinQuality" ) {
    return veinQuality;
  }
  else if ( name == "veinWave" ) {
    return veinWave;
  }
  else if ( name == "softNoise" ) {
    return softNoise;
  }
  else {
    return ReProceduralNoise::getNamedValue(name);
  }
};

void ReMarble::serialize( QDataStream& dataStream ) {
  ReProceduralNoise::serialize(dataStream);
  dataStream << depth << turbulence
             << (quint16) veinQuality << (quint16) veinWave
             << softNoise;
};


void ReMarble::deserialize( QDataStream& dataStream ) {
  ReProceduralNoise::deserialize(dataStream);
  quint16 veinQualityInt, veinWaveInt;
  dataStream >> depth >> turbulence
             >> veinQualityInt >> veinWaveInt
             >> softNoise;
  veinQuality = static_cast<VeinQuality>(veinQualityInt);
  veinWave = static_cast<VeinWave>(veinWaveInt);
};


} // namespace
