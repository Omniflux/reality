/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReGrayscale.h"
#include "ReTools.h"
#include "ReMaterials.h"
#include "ReDefs.h"
#include "ReTextureCreator.h"
#include "ReSceneDataGlobal.h"

namespace Reality {

ReGrayscale::ReGrayscale( const QString name, ReTextureContainer* parentMat ) : 
  Re2DTexture(name, parentMat),
  color(255,255,255) 
{
  type = TexGrayscale;
  textureDataType = ReTexture::color;
  setChannel(
    "tex",
    ImageMapPtr( new ImageMap(QString("%1_baseMap").arg(name), parentMat) )
  );
};

ReGrayscale::ReGrayscale( const QString name, 
                          ReTextureContainer* parentMat, 
                          const int rgbChannel ) :
  Re2DTexture(name, parentMat),
  rgbChannel((RGBChannel)rgbChannel),
  color(255,255,255) 
{
  type = TexGrayscale;
  textureDataType = ReTexture::color;
  setChannel(
    "tex",
    ImageMapPtr( new ImageMap(QString("%1_baseMap").arg(name), parentMat) )
  );
};

// Texture conversion ctor
ReGrayscale::ReGrayscale( const ReTexturePtr srcTex ) :
  Re2DTexture(srcTex),
  color(255,255,255)
{
  type = TexGrayscale;
  textureDataType = ReTexture::color;
  ReTexturePtr newTex;
  // Create a duplicate of the source texture with a new name.
  // If the source tex is of type Grayscale then duplicate the
  // sub texture instead
  if (srcTex->getType() == TexGrayscale) {
    newTex = srcTex.staticCast<ReGrayscale>()->getTexture();
  }
  else {
    newTex = ReTexturePtr( 
      TextureCreator::createTexture(
        QString("%1_%2").arg(name).arg("tex"), srcTex
      )
    );
  }
  setChannel("tex", newTex);
}

ReGrayscale::ReGrayscale( const QString name,
                          ReTextureContainer* parentMat, 
                          const int rgbChannel, 
                          const ReTexturePtr texture,
                          const QColor color ) :
  Re2DTexture(name, parentMat),
  rgbChannel((RGBChannel)rgbChannel),
  color(color) 
{
  textureDataType = ReTexture::color;
  type = TexGrayscale;
  setChannel("tex", texture);
};


RGBChannel ReGrayscale::getRgbChannel() const {
  return rgbChannel;
};

void ReGrayscale::setRgbChannel( RGBChannel newVal ) {
  rgbChannel = newVal;
};

const QString ReGrayscale::getRGBChannelAsString() const {
  switch(rgbChannel) {
    case RGB_Red:
      return "red";
    case RGB_Green:
      return "green";
    case RGB_Blue:
      return "blue";
    case RGB_Mean:
      return "mean";
  }
  return "mean";
}


void ReGrayscale::setTexture( ReTexturePtr newVal ) {
  setChannel("tex", newVal);
};

void ReGrayscale::setNamedValue( const QString& vName, const QVariant& value ) {
  if (vName == "texture") {
    setChannel("tex",value.value<ReTexturePtr>());
  }
  else if (vName == "color") {
     color = value.value<QColor>();
  }
  else if (vName == "rgbChannel") {
    rgbChannel = static_cast<RGBChannel>(value.toInt());
  }
  else if (
            vName == "gain"    || vName == "gamma" || vName == "uOffset" ||
            vName == "vOffset" || vName == "uTile" || vName == "vTile"   ||
            vName == "mapping"
          )
  {
    channels.value("tex")->setNamedValue(vName, value);
  }
  else {
    Re2DTexture::setNamedValue(vName, value);
  }
}

const QVariant ReGrayscale::getNamedValue( const QString& vName ) {
  if (vName == "texture") {
    val.setValue<ReTexturePtr>(channels.value("tex"));
  }
  else if (vName == "color") {
    val = color;
  }
  else if (vName == "rgbChannel") {
    val = rgbChannel;
  }
  else if (vName == "gain") {
    auto tex = channels.value("tex");
    if (!tex.isNull()) {
      return tex->getNamedValue(vName);
    }
    return 1.0;
  }
  else if (vName == "gamma") {
    auto tex = channels.value("tex");
    if (!tex.isNull()) {
      return tex->getNamedValue(vName);
    }
    return RealitySceneData->getGamma();
  }
  else {
    return Re2DTexture::getNamedValue(vName);
  }
  return val;
}

const QColor ReGrayscale::getColor() const {
  return color;
};

void ReGrayscale::setColor( QColor newVal ) {
  color = newVal;
};

QStringList ReGrayscale::getDependencies() {
  deps.clear();
  if (!channels["tex"].isNull()) {
    deps << channels["tex"]->getName();
  }
  return deps;
};

void ReGrayscale::replaceInnerTexture( const QString textureName, 
                                       ReTexturePtr newTexture ) 
{
  if ( channels["tex"]->getName() == textureName ) {
    setChannel("tex", newTexture);
  }  
}

void ReGrayscale::reparent( ReTextureContainer* newParent ) {
  Re2DTexture::reparent(newParent);
  auto t = channels["tex"];
  if (!t.isNull()) {
    t->reparent(parent);
    parent->addTextureToCatalog(t);
  }
}

void ReGrayscale::serialize( QDataStream& dataStream ) {
  ReTexture::serialize(dataStream);
  dataStream << (quint16) rgbChannel << color;
  serializeChannels(dataStream);
};

void ReGrayscale::deserialize( QDataStream& dataStream ) {
  ReTexture::deserialize(dataStream);
  quint16 channel;
  dataStream >> channel >> color;
  rgbChannel = static_cast<RGBChannel>(channel);
  deserializeChannels(dataStream, parent);
};



}