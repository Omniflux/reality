/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReFBM.h"
#include "ReTools.h"

namespace Reality {

ReFBM::ReFBM(const QString name, ReTextureContainer* parentMat ) : 
  ReTexture3D(name, parentMat),
  octaves(6),
  roughness(0.5),
  wrinkled(false)
{
  type = TexFBM;
  textureDataType = numeric;
};

// Conversion ctor
ReFBM::ReFBM( const ReTexturePtr srcTex ) :
  ReTexture3D(srcTex),
  octaves(6),
  roughness(0.5),
  wrinkled(false)
{
  type = TexFBM;
  textureDataType = numeric;

  if ( srcTex->getType() == TexFBM ) {
    ReFBMPtr t2 = srcTex.staticCast<ReFBM>();
    octaves   = t2->octaves;
    roughness = t2->roughness;
    wrinkled  = t2->wrinkled;
  }
}

quint16 ReFBM::getOctaves() const {
  return octaves;
};

void ReFBM::setOctaves( quint16 newVal ) {
  octaves = newVal;
};

float ReFBM::getRoughness() const {
  return roughness;
};

void ReFBM::setRoughness( float newVal ) {
  roughness = newVal;
};

void ReFBM::setNamedValue( const QString& name , const QVariant& value ) {
  if (name == "octaves") {
    octaves = value.toInt();
  }
  else if (name == "roughness") {
    roughness = value.toFloat();
  }
  else if (name == "wrinkled") {
    wrinkled = value.toBool();
  }
  else {
    ReTexture3D::setNamedValue(name, value);
  }
};

const QVariant ReFBM::getNamedValue( const QString& name ) {
  QVariant val;
  if (name == "octaves") {
    return octaves;
  }
  else if (name == "roughness") {
    return roughness;
  }
  else if (name == "wrinkled") {
    return wrinkled;
  }
  else {
    return ReTexture3D::getNamedValue(name);
  }
  return val;
};

bool ReFBM::isWrinkled() const {
  return wrinkled;
};
void ReFBM::setWrinkled( bool newVal ) {
  wrinkled = newVal;
};

void ReFBM::serialize( QDataStream& dataStream ) {
  ReTexture3D::serialize(dataStream);
  dataStream << octaves << roughness << wrinkled;
};


void ReFBM::deserialize( QDataStream& dataStream ) {
  ReTexture3D::deserialize(dataStream);
  dataStream >> octaves >> roughness >> wrinkled;
};



} // namespace
