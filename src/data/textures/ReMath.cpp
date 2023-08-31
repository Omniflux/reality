/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReMath.h"

#include "ReTexture.h"
#include "ReTextureContainer.h"
#include "ReTextureCreator.h"
#include "textures/ReConstant.h"
#include "textures/ReMix.h"


namespace Reality {

ReMath::ReMath( const QString name, ReTextureContainer* parentMat ) : 
  Re2DTexture(name, parentMat), 
  amountTex1(1.0),
  amountTex2(1.0),
  function(multiply)
{
  type = TexMath;
  textureDataType = ReTexture::numeric;
  initChannels();
}

ReMath::ReMath( const QString name,
                ReTextureContainer* parentMat,
                ReTexturePtr tex1, 
                ReTexturePtr tex2, 
                functions func) :

  Re2DTexture(name, parentMat),
  amountTex1(1.0),
  amountTex2(1.0),
  function(func)
{  
  type = TexMath;
  textureDataType = ReTexture::numeric;
  channels[MT_TEX1] = tex1;
  channels[MT_TEX2] = tex2;
  if (parent) {
    parent->addTextureToCatalog(tex1);
    parent->addTextureToCatalog(tex2);
  }
};

// Texture conversion ctor
ReMath::ReMath( const ReTexturePtr srcTex ) :
  Re2DTexture(srcTex),
  amountTex1(1.0),
  amountTex2(1.0)
{  
  type = TexMath;
  textureDataType = ReTexture::numeric;
  function = add;
  // duplicate ctor?
  switch(srcTex->getType()) {
    case TexMath: {
      ReMathPtr t2 = srcTex.staticCast<ReMath>();
      channels[MT_TEX1] = t2->channels[MT_TEX1];
      channels[MT_TEX2] = t2->channels[MT_TEX2];
      function = t2->function;
      break;
    }
    case TexMix: {
      ReMixTexturePtr t2 = srcTex.staticCast<ReMixTexture>();
      channels[MT_TEX1] = t2->getTexture1();
      channels[MT_TEX2] = t2->getTexture2();
      function = add;
      break;
    }
    default: {
      if (srcTex->getDataType() == numeric) {
        channels[MT_TEX1] = ReTexturePtr( 
          ReTextureCreator::createTexture(
            QString("%1_%2").arg(name).arg("tex1"), 
            srcTex)
        );
        channels[MT_TEX2] = ReTexturePtr(
          new ReConstant(QString("%1_%2").arg(name).arg("tex2"), parent, 1.0f)
        );
        function = add;
        if (parent) {
          parent->addTextureToCatalog(channels[MT_TEX1]);
          parent->addTextureToCatalog(channels[MT_TEX2]);
        }  
      }
      else {
        initChannels();
      }
    }
  }
}  

/**
 * Initializes the channels for this compound texture. Here we need to pay attention
 * to what happens when the texture is created during serialization/deserialization.
 * When deserializing we first deserialize the contained textures, what goes in channels
 * MT_TEX1 and MT_TEX2. Then we create a blank Math texture in the UI. If, at this point,
 * we blindingly create new inner textures, with the same template name, we would be 
 * overwriting the existing textures deserialized from the host. 
 *
 * So, we need to check if the textures exist already, because they were deserialized, 
 * and in that case we simply link to them. 
 *
 * If no existing textures are found then we create new ones with default values.
 */
void ReMath::initChannels() {
  bool addedT1, addedT2;
  addedT1 = addedT2 = false;

  QString t1Name = QString("%1_%2").arg(name).arg("tex1");
  QString t2Name = QString("%1_%2").arg(name).arg("tex2");
  if (parent) {
    auto t1 = parent->getTexture(t1Name);
    if (!t1.isNull()) {
      channels[MT_TEX1] = t1;
    }
    auto t2 = parent->getTexture(t2Name);
    if (!t2.isNull()) {
      channels[MT_TEX2] = t2;
    }
  }

  if (!channels.contains(MT_TEX1) || channels[MT_TEX1].isNull() ) {    
    channels[MT_TEX1] = ReTexturePtr( new ReConstant( t1Name, parent, 0.0f) );
    addedT1 = true;
  }
  if (!channels.contains(MT_TEX2) || channels[MT_TEX2].isNull() ) {
    channels[MT_TEX2] = ReTexturePtr( new ReConstant(t2Name, parent, 1.0f) );
    addedT2 = true;
  }
  if (parent) {
    if (addedT1) {
      parent->addTextureToCatalog(channels[MT_TEX1]);
    }
    if (addedT2) {
      parent->addTextureToCatalog(channels[MT_TEX2]);
    }
  }  
}

QString ReMath::toString() {
  return QString("Reality::ReMath(%1): %2({Tex1: %3} {Tex2: %4})")
           .arg(name)
           .arg(getFunctionAsString())
           .arg(channels[MT_TEX1] ? channels[MT_TEX1]->toString() : "")
           .arg(channels[MT_TEX2] ? channels[MT_TEX2]->toString() : "");
}

void ReMath::replaceInnerTexture( const QString textureName, ReTexturePtr newTexture ) {
  if ( channels[MT_TEX1]->getName() == textureName ) {
    channels[MT_TEX1].clear();
    channels[MT_TEX1] = newTexture;
  }  
  if ( channels[MT_TEX2]->getName() == textureName ) {
    channels[MT_TEX2].clear();
    channels[MT_TEX2] = newTexture;
  }  
}

void ReMath::serialize( QDataStream& dataStream ) {
  Re2DTexture::serialize(dataStream);
  dataStream << (quint16)function << amountTex1 << amountTex2;
  serializeChannels(dataStream);
};

void ReMath::deserialize( QDataStream& dataStream ) {
  Re2DTexture::deserialize(dataStream);
  quint16 f;
  dataStream >> f >> amountTex1 >> amountTex2;
  function = static_cast<functions>(f);
  deserializeChannels(dataStream, parent);
};

void ReMath::reparent( ReTextureContainer* parentMat ) {
  Re2DTexture::reparent(parentMat);
  if (!channels[MT_TEX1].isNull()) {
    channels[MT_TEX1]->reparent(parentMat);
    parentMat->addTextureToCatalog(channels[MT_TEX1]);
  }
  if (!channels[MT_TEX2].isNull()) {
    channels[MT_TEX2]->reparent(parentMat);
    parentMat->addTextureToCatalog(channels[MT_TEX2]);
  }
}

void ReMath::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "texture1") {
    channels[MT_TEX1] = value.value<ReTexturePtr>();
  }
  else if (vname == "texture2") {
    channels[MT_TEX2] = value.value<ReTexturePtr>();
  }
  else if (vname == "amount1") {
    amountTex1 = value.toFloat();
  }
  else if (vname == "amount2") {
    amountTex2 = value.toFloat();
  }
  else if (vname == "function") {
    function = static_cast<ReMath::functions>(value.toInt());
  }
  else {
    Re2DTexture::setNamedValue(vname, value);
  }
}

const QVariant ReMath::getNamedValue( const QString& vname ) {
  if (vname == "texture1") {
    val.setValue(channels[MT_TEX1]);
  }
  else if (vname == "texture2") {
    val.setValue(channels[MT_TEX2]);
  }
  else if (vname == "amount1") {
    val = amountTex1;
  }
  else if (vname == "amount2") {
    val = amountTex2;
  }
  else if (vname == "function") {
    val = function;
  }
  else {
    return Re2DTexture::getNamedValue(vname);
  }
  return val;
}

}