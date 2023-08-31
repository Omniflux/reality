/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReTexture.h"

#include "ReTextureContainer.h"


using namespace Reality;

//! Conversion table used in <ReTexture::getTypeAsString()> to return a string version of
//! the type name of each texture.
const char* ReTextureTypesAsStrings[TexUndefined+1] = {
  "ImageMap",
  "Constant",
  "ColorMath",
  "Math",
  "Band",
  "Bricks",
  "Checkers",
  "Clouds",
  "FBM",
  "FresnelColor",
  "Marble",
  "Mix",
  "MultiMix",
  "DistortedNoise",
  "Wood",
  "InvertMap",
  "Grayscale",
  "Undefined"
}; 

const QString ReTexture::getTypeAsString() const {
  return(ReTextureTypesAsStrings[type]);
}

const QString ReTexture::getTypeAsString( const ReTextureType texType ) {
  return(ReTextureTypesAsStrings[texType]);
}

const QString ReTexture::getDataTypeAsString( const ReTextureDataType dataType ) {
  return(dataTypeNames[dataType]);  
}

QString ReTexture::dataTypeNames[fresnel+1] = { 
  QString("color"),  
  QString("float"),
  QString("fresnel")
};


ReTexture::ReTexture(const QString& name, ReTextureContainer* parentMat) : 
  name(name),
  type(TexUndefined),
  textureDataType(color)
{
  reparent(parentMat);
}

ReTexture::ReTexture( ReTexturePtr srcTex ) :
  name(srcTex->name),
  textureDataType(srcTex->textureDataType)
{
  reparent(srcTex->parent);
}

const QString ReTexture::getUniqueName() const {
  if (!parent) {
    return name;
  }
  QString pname = parent->getUniqueName();
  return QString("%1:%2")
           .arg(pname.replace(" ","_"))
           .arg(name);
}

void ReTexture::reparent( ReTextureContainer* newParent ) {
  parent = newParent;
}

void ReTexture::serialize( QDataStream& dataStream ) {
  dataStream << (quint16) type << name << (quint16)textureDataType;
};

void ReTexture::deserialize( QDataStream& dataStream ) {
  quint16 typeInt;
  quint16 dataTypeInt;
  dataStream >> typeInt >> name >> dataTypeInt;
  type = static_cast<ReTextureType>(typeInt);
  textureDataType = static_cast<ReTextureDataType>(dataTypeInt);
};


void ReTexture3D::serialize( QDataStream& dataStream ) {
  ReTexture::serialize(dataStream);
  dataStream << scale
             << xRot << yRot << zRot
             << (quint16) mapping;
};

void ReTexture3D::deserialize( QDataStream& dataStream ) {
  ReTexture::deserialize(dataStream);
  quint16 mappingInt;
  dataStream >> scale
             >> xRot >> yRot >> zRot
             >> mappingInt;
  mapping = static_cast<Mapping3D>(mappingInt);
};


