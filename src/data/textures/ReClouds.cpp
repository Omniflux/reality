/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReClouds.h"


namespace Reality {

// Constructor: Clouds
Clouds::Clouds( const QString name, ReTextureContainer* parentMat ) : 
  ReProceduralNoise(name, parentMat),
  usesHardNoiseFlag(false),
  noiseDepth(1) {

  textureDataType = numeric;
  type = TexClouds;
};

// Texture conversion ctor
Clouds::Clouds( const ReTexturePtr srcTex ) :
  ReProceduralNoise(srcTex),
  usesHardNoiseFlag(false),
  noiseDepth(1) 
{
  textureDataType = numeric;
  type = TexClouds;

  switch ( srcTex->getType() ) {
    case TexClouds: {
      CloudsPtr t2 = srcTex.staticCast<Clouds>();
      usesHardNoiseFlag = t2->usesHardNoiseFlag;
      noiseDepth        = t2->noiseDepth;
      break;
    }
    case TexMath: {
      auto t1 = srcTex->getNamedValue("texture1").value<ReTexturePtr>();
      if (t1->getType() == TexClouds) {
        copyProperties(t1);
      }
      break;
    }
    case TexMix: {
      auto t1 = srcTex->getNamedValue("tex1").value<ReTexturePtr>();
      if (t1->getType() == TexClouds) {
        copyProperties(t1);
      }
      break;
    }
    default:
      break;
  }
}

void Clouds::copyProperties( const ReTexturePtr srcTex ) {
  setNamedValue("noiseType", srcTex->getNamedValue("noiseType"));
  setNamedValue("noiseDepth", srcTex->getNamedValue("noiseDepth"));
  setNamedValue("usesHardNoise", srcTex->getNamedValue("usesHardNoise"));
  ReProceduralNoise::copyProperties(srcTex);
}

void Clouds::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "noiseType" || name == "noiseBasis") {
    setNoiseBasis(static_cast<Reality::ReProceduralNoise::NoiseDistortionType>(value.toInt()));
  }
  else if (name == "noiseDepth") {
    setNoiseDepth(value.toInt());
  }
  else if (name == "usesHardNoise") {
    setHardNoise(value.toBool());
  }
  else {
    ReProceduralNoise::setNamedValue(name, value);
  }
}

const QVariant Clouds::getNamedValue( const QString& name ) {
  if (name == "noiseType" || name == "noiseBasis") {
    val = getNoiseBasis();
  }
  else if (name == "noiseDepth") {
    val = getNoiseDepth();
  }
  else if (name == "usesHardNoise") {
    val = usesHardNoise();
  }
  else {
    return ReProceduralNoise::getNamedValue(name);
  }
  return val;
}

void Clouds::serialize( QDataStream& dataStream ) {
  ReProceduralNoise::serialize(dataStream);
  dataStream << usesHardNoiseFlag << noiseDepth;
};


void Clouds::deserialize( QDataStream& dataStream ) {
  ReProceduralNoise::deserialize(dataStream);
  dataStream >> usesHardNoiseFlag >> noiseDepth;
};


}