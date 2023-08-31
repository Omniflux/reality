/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReFresnelColor.h"

#include "ReTextureCreator.h"


namespace Reality {


QString ReFresnelColor::toString() {
  return "ReFresnelColor texture";
};



void ReFresnelColor::reparent( ReTextureContainer* parentMat ) {
  ReTexture::reparent(parentMat);
  if (!tex.isNull()) {
    tex->reparent(parentMat);
    parentMat->addTextureToCatalog(tex);
  }
}

QStringList ReFresnelColor::getDependencies() {
  deps.clear();
  if (!tex.isNull()) {
    deps << tex->getName();
  }
  return deps;
};

void ReFresnelColor::serialize( QDataStream& dataStream ) {
  ReTexture::serialize(dataStream);
  dataStream << color;
  if (tex.isNull()) {
    dataStream << false;
  }
  else {
    dataStream << true;
    tex->serialize(dataStream);
  }
};

void ReFresnelColor::deserialize( QDataStream& dataStream ) {
  ReTexture::deserialize(dataStream);
  dataStream >> color;
  bool texturePresent;
  dataStream >> texturePresent;
  if (texturePresent) {
    tex = ReTextureCreator::deserialize(dataStream, parent);
    parent->addTextureToCatalog(tex);
  }
};




} // namespace