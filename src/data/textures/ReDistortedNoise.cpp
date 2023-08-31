/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReDistortedNoise.h"


namespace Reality {

// Constructor: ReDistortedNoise
ReDistortedNoise::ReDistortedNoise(const QString name, ReTextureContainer* parentMat ) : 
  ReProceduralNoise(name, parentMat),
  distortionType(IMPROVED_PERLIN),
  distortionAmount(1.0)
{
  type            = TexDistortedNoise;
  textureDataType = numeric;
};

// Texture conversion ctor
ReDistortedNoise::ReDistortedNoise( const ReTexturePtr srcTex ) :
  ReProceduralNoise(srcTex),
  distortionType(IMPROVED_PERLIN),
  distortionAmount(1.0)
{
  type            = TexDistortedNoise;
  textureDataType = numeric;

  if ( srcTex->getType() == TexDistortedNoise ) {
    ReDistortedNoisePtr t2 = srcTex.staticCast<ReDistortedNoise>();
    distortionType   = t2->distortionType;
    distortionAmount = t2->distortionAmount;
  }
}

void ReDistortedNoise::serialize( QDataStream& dataStream ) {
  ReProceduralNoise::serialize(dataStream);
  dataStream << distortionAmount << (quint16) distortionType;
};


void ReDistortedNoise::deserialize( QDataStream& dataStream ) {
  ReProceduralNoise::deserialize(dataStream);
  quint16 tmpType;
  dataStream >> distortionAmount >> tmpType;
  distortionType = (NoiseDistortionType) tmpType;
};

void ReDistortedNoise::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "distortionType") {
    distortionType = static_cast<NoiseDistortionType>(value.toInt());
  }
  else if (name == "distortionAmount") {
    distortionAmount = value.toFloat();
  }
  else {
    ReProceduralNoise::setNamedValue(name, value);
  }
}

const QVariant ReDistortedNoise::getNamedValue( const QString& name ) {
  if (name == "distortionType") {
    val = distortionType;
  }
  else if (name == "distortionAmount") {
    val = distortionAmount;
  }
  else {
    val = ReProceduralNoise::getNamedValue(name);
  }
  return val;
}

}