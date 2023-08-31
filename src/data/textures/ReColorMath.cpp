/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReColorMath.h"

#include "ReTextureContainer.h"
#include "ReTextureCreator.h"
#include "textures/ReConstant.h"


namespace Reality {

ReColorMath::ReColorMath( const QString name, ReTextureContainer* parentMat ) : 
  Re2DTexture(name, parentMat) 
{  
  type = TexColorMath;
  function = multiply;
  color1 = RE_PURE_WHITE_COLOR;
  color2 = RE_PURE_WHITE_COLOR;
  textureDataType = color;
  channels[CM_TEX1] = ReTexturePtr();
  channels[CM_TEX2] = ReTexturePtr();
}

ReColorMath::ReColorMath( const QString name, 
                          ReTextureContainer* parentMat,
                          ReTexturePtr tex1, 
                          ReTexturePtr tex2, 
                          functions func ) :
  Re2DTexture(name,parentMat),
  function(func) 
{
  type            = TexColorMath;
  textureDataType = color;
  color1 = RE_PURE_WHITE_COLOR;
  color2 = RE_PURE_WHITE_COLOR;
  channels[CM_TEX1] = tex1;
  channels[CM_TEX2] = tex2;
  if (parent) {
    parent->addTextureToCatalog(channels[CM_TEX1]);
    parent->addTextureToCatalog(channels[CM_TEX2]);
  }
};

// Just a color and a texture. No second texture and no function.
ReColorMath::ReColorMath( const QString name, 
                          ReTextureContainer* parentMat,
                          QColor col1, 
                          ReTexturePtr tex1 ) :
  Re2DTexture(name,parentMat),
  color1(col1)
{
  type     = TexColorMath;
  textureDataType = color;
  function = none;
  color2 = RE_PURE_WHITE_COLOR;
  channels[CM_TEX1] = tex1;
  channels[CM_TEX2] = ReTexturePtr();
  if (parent) {
    parent->addTextureToCatalog(channels[CM_TEX1]);
  }
};

// Texture conversion ctor
ReColorMath::ReColorMath( const ReTexturePtr srcTex ) :
  Re2DTexture(srcTex)
{
  // Basic initialization
  type            = TexColorMath;
  textureDataType = color;
  function        = multiply;
  // duplicate ctor?
  if ( srcTex->getType() == TexColorMath ) {
    ReColorMathPtr t2 = srcTex.staticCast<ReColorMath>();
    color1   = t2->color1;
    color2   = t2->color2;
    channels[CM_TEX1] = t2->channels[CM_TEX1];
    channels[CM_TEX2] = t2->channels[CM_TEX2];
    function = t2->function;
  }
  else {
    // Conversion from some other kind of texture. We just stick a copy of the 
    // original texture in the first channel
    color1 = RE_PURE_WHITE_COLOR;
    color2 = RE_PURE_WHITE_COLOR;
    function = none;
    channels[CM_TEX1] = ReTexturePtr( 
      ReTextureCreator::createTexture(
        QString("%1_%2").arg(name).arg("tex1"), srcTex
      )
    );
    parent->addTextureToCatalog(channels[CM_TEX1]);
    channels[CM_TEX2] = ReTexturePtr();
    if (parent) {
      parent->addTextureToCatalog(channels[CM_TEX2]);
    }
  }
}

void ReColorMath::setTextureDataType( ReTextureDataType newVal ) {
  textureDataType = newVal;
  // texture1->setTextureDataType(newVal);
  // texture2->setTextureDataType(newVal);
}

void ReColorMath::reparent( ReTextureContainer* parentMat ) {
  Re2DTexture::reparent(parentMat);
  if (!channels[CM_TEX1].isNull()) {
    channels[CM_TEX1]->reparent(parentMat);
    parentMat->addTextureToCatalog(channels[CM_TEX1]);
  }
  if (!channels[CM_TEX2].isNull()) {
    channels[CM_TEX2]->reparent(parentMat);
    parentMat->addTextureToCatalog(channels[CM_TEX2]);
  }
}


QString ReColorMath::toString() {
  return QString("Reality::ReColorMath(%1) {Tex1: %2} {Tex2: %3} Func: %4")
           .arg(name)
           .arg(channels[CM_TEX1] ? channels[CM_TEX1]->toString() : "")
           .arg(channels[CM_TEX2] ? channels[CM_TEX2]->toString() : "")
           .arg(function);
}

void ReColorMath::setTexture1( QColor& clr ) {
  channels[CM_TEX1] = ReTexturePtr(new ReConstant(QString("%1_tex1").arg(name), parent, clr ));
  channels[CM_TEX1]->setTextureDataType(getDataType());
};

void ReColorMath::setTexture2( QColor& clr ) {
  channels[CM_TEX2] = ReTexturePtr(new ReConstant(QString("%1_tex2").arg(name), parent, clr));
  channels[CM_TEX2]->setTextureDataType(getDataType());
};


void ReColorMath::replaceInnerTexture( const QString textureName, ReTexturePtr newTexture ) {
  if ( !channels[CM_TEX1].isNull() && channels[CM_TEX1]->getName() == textureName ) {
    channels[CM_TEX1].clear();
    channels[CM_TEX1] = newTexture;
  }  
  if ( !channels[CM_TEX2].isNull() && channels[CM_TEX2]->getName() == textureName ) {
    channels[CM_TEX2].clear();
    channels[CM_TEX2] = newTexture;
  }  
}


void ReColorMath::serialize( QDataStream& dataStream ) {
  Re2DTexture::serialize(dataStream);
  dataStream << (quint16)function << color1 << color2;
  serializeChannels(dataStream);
};

void ReColorMath::deserialize( QDataStream& dataStream ) {
  Re2DTexture::deserialize(dataStream);
  quint16 f;
  dataStream >> f >> color1 >> color2;
  function = static_cast<functions>(f);
  deserializeChannels(dataStream, parent);
};


void ReColorMath::setNamedValue( const QString& vname, const QVariant& value ) {

  if (vname == "texture1") {
    channels[CM_TEX1] = value.value<ReTexturePtr>();
  }
  if (vname == "color1") {
    color1 = value.value<QColor>();
  }
  else if (vname == "texture2") {
    channels[CM_TEX2] = value.value<ReTexturePtr>();
  }
  if (vname == "color2") {
    color2 = value.value<QColor>();
  }
  else if (vname == "function") {
    function = static_cast<functions>(value.toInt());
  }
  else {
    Re2DTexture::setNamedValue(vname, value);
  }
}

const QVariant ReColorMath::getNamedValue( const QString& propName ) {
  if (propName == "texture1") {
    val.setValue(channels.value(CM_TEX1));
  }
  else if (propName == "color1") {
    val.setValue(color1);
  }
  else if (propName == "texture2") {
    val.setValue(channels.value(CM_TEX2));
  }
  else if (propName == "color2") {
    val.setValue(color2);
  }
  else if (propName == "function") {
    val = function;
  }
  else {
    return Re2DTexture::getNamedValue(propName);
  }
  return val;
}

}