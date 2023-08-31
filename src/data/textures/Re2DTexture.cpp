/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/Re2DTexture.h"


namespace Reality {
  
void Re2DTexture::serialize( QDataStream& dataStream ) {
  ReTexture::serialize(dataStream);
  dataStream << uTile << vTile << uOffset << vOffset << (quint16) mapping;
};



void Re2DTexture::deserialize( QDataStream& dataStream ) {
  ReTexture::deserialize(dataStream);
  dataStream >> uTile >> vTile >> uOffset >> vOffset;
  quint16 mappingInt;
  dataStream >> mappingInt;
  mapping = static_cast<Mapping2D>(mappingInt);
};

void Re2DTexture::copyProperties( const ReTexturePtr srcTex ) {
  setNamedValue("uTile", srcTex->getNamedValue("uTile"));
  setNamedValue("vTile", srcTex->getNamedValue("vTile"));
  setNamedValue("uOffset", srcTex->getNamedValue("uOffset"));  
  setNamedValue("vOffset", srcTex->getNamedValue("vOffset"));  
  setNamedValue("mapping", srcTex->getNamedValue("mapping"));  
}

void Re2DTexture::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "uTile") {
    uTile = value.toDouble();
  }
  else if (vname == "vTile") {
    vTile = value.toDouble();
  }
  else if (vname == "uOffset") {
    uOffset = value.toDouble();
  }
  else if (vname == "vOffset") {
    vOffset = value.toDouble();
  }
  else if (vname == "mapping") {
    mapping = static_cast<Mapping2D>(value.toInt());
  }
  else {
    ReTexture::setNamedValue(vname, value);
  }
}

const QVariant Re2DTexture::getNamedValue( const QString& vname ) {
  if (vname == "uTile") {
    return uTile;
  }
  else if (vname == "vTile") {
    return vTile;
  }
  else if (vname == "uOffset") {
    return uOffset;
  }
  else if (vname == "vOffset") {
    return vOffset;
  }
  else if (vname == "mapping") {
    return mapping;
  }
  return ReTexture::getNamedValue(vname);
}

}