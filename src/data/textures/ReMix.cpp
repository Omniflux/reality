/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReMix.h"

#include "ReTextureContainer.h"
#include "ReTextureCreator.h"
#include "textures/ReConstant.h"
#include "textures/ReImageMap.h"


namespace Reality {

#define RE_MIXT_TEX1 "tex1"
#define RE_MIXT_TEX2 "tex2"
#define RE_MIXT_MIXER "mixtex"

ReMixTexture::ReMixTexture( const QString name, 
                            ReTextureContainer* parentMat,
                            const ReTextureDataType dataType ) : 
  ReTexture(name, parentMat), 
  mixAmount(0.5),
  fTex1(0),
  fTex2(0),
  useFloatValuesFlag(false)
{
  type = TexMix;
  textureDataType = dataType;
  initChannels();
};

// Texture conversion ctor
ReMixTexture::ReMixTexture( const ReTexturePtr srcTex ) :
  ReTexture(srcTex),
  mixAmount(0.5),
  fTex1(0),
  fTex2(0),
  useFloatValuesFlag(false)
{
  type = TexMix;
  // duplicate ctor?
  switch( srcTex->getType() ) {
    case TexMix: {
      ReMixTexturePtr t2 = srcTex.staticCast<ReMixTexture>();
      channels[RE_MIXT_TEX1]  = t2->channels[RE_MIXT_TEX1];
      channels[RE_MIXT_TEX2]  = t2->channels[RE_MIXT_TEX2];
      channels[RE_MIXT_MIXER] = t2->channels[RE_MIXT_MIXER];
      break;
    }
    case TexColorMath: {
      ReTexturePtr t1 = ReTexturePtr( 
        ReTextureCreator::createTexture(QString("%1_%2").arg(name).arg(RE_MIXT_TEX1),
          srcTex->getNamedValue("texture1").value<ReTexturePtr>())
      );
      parent->addTextureToCatalog(t1);
      t1->reparent(parent);
      channels[RE_MIXT_TEX1] = t1;

      ReTexturePtr cmTex2 = srcTex->getNamedValue("texture2").value<ReTexturePtr>();
      ReTexturePtr t2;
      if (cmTex2.isNull()) {
        t2 = ReTexturePtr( 
          new ReConstant( QString("%1_tex2").arg(name), parent, RE_WHITE_COLOR )
        );        
      }
      else {
        t2 = ReTexturePtr( 
          ReTextureCreator::createTexture(QString("%1_%2").arg(name).arg(RE_MIXT_TEX2), cmTex2)
        );
      }
      parent->addTextureToCatalog(t2);
      t2->reparent(parent);
      channels[RE_MIXT_TEX2] = t2;

      break;
    }
    default:  
      initChannels(srcTex);  
  }
}

// Destructor: ReMixTexture
ReMixTexture::~ReMixTexture() {
  channels[RE_MIXT_TEX1].clear();
  channels[RE_MIXT_TEX2].clear();
  channels[RE_MIXT_MIXER].clear();
};

void ReMixTexture::initChannels( const ReTexturePtr baseTex ) {
  QString tex1Name = QString("%1_tex1").arg(name);
  QString tex2Name = QString("%1_tex2").arg(name);
  QString mixTexName = QString("%1_mix").arg(name);

  if (baseTex.isNull() && parent) {
    // If the textures exist already, as in the case of serializing/
    // deserializing, then we simply connect to them
    auto tex1 = parent->getTexture(tex1Name);
    if (!tex1.isNull()) {
      channels[RE_MIXT_TEX1] = tex1;
    }
    auto tex2 = parent->getTexture(tex2Name);
    if (!tex2.isNull()) {
      channels[RE_MIXT_TEX2] = tex2;
    }
    auto mixTex = parent->getTexture(mixTexName);
    if (!mixTex.isNull()) {
      channels[RE_MIXT_MIXER] = mixTex;
    }
    if ( !tex1.isNull() && !tex2.isNull() ) {
      // we're done here
      return;
    }
  }

  if ( textureDataType == color ) {
    // otherwise we create default textures
    if (baseTex.isNull()) {
      channels[RE_MIXT_TEX1] = ReTexturePtr( 
        new ReConstant( tex1Name, parent, RE_BLACK_COLOR )
      );      
    }
    else {
      channels[RE_MIXT_TEX1] = ReTexturePtr( 
        ReTextureCreator::createTexture(tex1Name, baseTex)
      );
    }
    channels[RE_MIXT_TEX2] = ReTexturePtr( 
      new ReConstant( tex2Name, parent, RE_WHITE_COLOR )
    );
  }
  else {
    channels[RE_MIXT_TEX1] = ReTexturePtr( 
      new ReConstant( tex1Name, parent, 5 )
    );
    channels[RE_MIXT_TEX2] = ReTexturePtr( 
      new ReConstant( tex2Name, parent, 240 )
    );
  }
  if (parent) {
    parent->addTextureToCatalog(channels[RE_MIXT_TEX1]);
    parent->addTextureToCatalog(channels[RE_MIXT_TEX2]);
  }

  channels[RE_MIXT_MIXER] = ReTexturePtr();
}

ReTexturePtr ReMixTexture::getTexture1() {
  return channels[RE_MIXT_TEX1];
};

void ReMixTexture::setTexture1( ReTexturePtr newVal ) {
  channels[RE_MIXT_TEX1] = newVal;
  if (!newVal.isNull()) {    
    if (parent) {
      newVal->reparent(parent);
      parent->addTextureToCatalog(newVal);
    }
    channels[RE_MIXT_TEX1]->setTextureDataType(textureDataType);
  }
};

ReTexturePtr ReMixTexture::getTexture2() {
  return channels[RE_MIXT_TEX2];
};

void ReMixTexture::setTexture2( ReTexturePtr newVal ) {
  channels[RE_MIXT_TEX2] = newVal;
  if (!newVal.isNull()) {    
    if (parent) {
      newVal->reparent(parent);
      parent->addTextureToCatalog(newVal);
    }
    channels[RE_MIXT_TEX2]->setTextureDataType(textureDataType);
  }
};

float ReMixTexture::getMixAmount() const {
  return mixAmount;
};
/*
 Method: setMixAmount
 */
void ReMixTexture::setMixAmount( const float newVal ) {
  mixAmount = newVal;
};

/*
 Method: getMixTexture
 */
ReTexturePtr ReMixTexture::getMixTexture() const {
  return channels[RE_MIXT_MIXER];
};

/*
 Method: setMixTexture
 */
void ReMixTexture::setMixTexture( const ReTexturePtr newVal ) {
  if (newVal.isNull()) {
    return;
  }
  channels[RE_MIXT_MIXER] = newVal;
  // If we pass a color texture for the mix texture then it needs to be converted to
  // numeric type. Since that is not possible for Lux we create a new numeric texture
  // based on the input one.
  if (newVal->getDataType() != ReTexture::numeric) {
    if (newVal->getType() == TexImageMap) {
      channels[RE_MIXT_MIXER] = ReTexturePtr( new ReImageMap(*newVal.staticCast<ReImageMap>()) );
      channels[RE_MIXT_MIXER]->setName(QString("%1_mixtexture").arg(name));
      channels[RE_MIXT_MIXER]->setTextureDataType(numeric);
    }
  }
  if (parent != NULL) {
    channels[RE_MIXT_MIXER]->reparent(parent);
    parent->addTextureToCatalog(channels[RE_MIXT_MIXER]);      
  }
};

// The mixed textures must be of the same type of the Mix texture
void ReMixTexture::setTextureDataType( ReTextureDataType newVal ) {
  textureDataType = newVal;
}

void ReMixTexture::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "texture1") {
    setTexture1(value.value<ReTexturePtr>());
  }
  else if (name == "texture2") {
    setTexture2(value.value<ReTexturePtr>());
  }
  else if (name == "mixTexture") {
    setMixTexture(value.value<ReTexturePtr>());
  }
  else if (name == "mixAmount") {
    setMixAmount(value.toDouble());
  }
  else {
    ReTexture::setNamedValue(name, value);
  }
}

const QVariant ReMixTexture::getNamedValue( const QString& name ) {
  // Add implementation
  if (name == "texture1") {
    val.setValue(channels[RE_MIXT_TEX1]);
    return val;
  }
  else if (name == "texture2") {
    val.setValue(channels[RE_MIXT_TEX2]);
    return val;
  }
  else if (name == "mixTexture") {
    val.setValue(channels[RE_MIXT_MIXER]);
    return val;
  }  
  else if (name == "mixAmount") {
    return mixAmount;
  }
  else {
    return ReTexture::getNamedValue(name);
  }
}

QStringList ReMixTexture::getDependencies() {
  deps.clear();
  if (channels[RE_MIXT_TEX1].isNull() || channels[RE_MIXT_TEX2].isNull()) {
    return deps;
  }
  deps << channels[RE_MIXT_TEX1]->getName() << channels[RE_MIXT_TEX2]->getName();
  if (!channels[RE_MIXT_MIXER].isNull()) {
    deps << channels[RE_MIXT_MIXER]->getName();
  }
  return deps;
};

void ReMixTexture::reparent( ReTextureContainer* newParent ) {
  ReTexture::reparent(newParent);
  if (!channels[RE_MIXT_TEX1].isNull()) {
    channels[RE_MIXT_TEX1]->reparent(parent);
    parent->addTextureToCatalog(channels[RE_MIXT_TEX1]);
  }
  if (!channels[RE_MIXT_TEX2].isNull()) {
    channels[RE_MIXT_TEX2]->reparent(parent);
    parent->addTextureToCatalog(channels[RE_MIXT_TEX2]);
  }
  if (!channels[RE_MIXT_MIXER].isNull()) {
    channels[RE_MIXT_MIXER]->reparent(parent);
    parent->addTextureToCatalog(channels[RE_MIXT_MIXER]);
  }
}

void ReMixTexture::replaceInnerTexture( const QString textureName, ReTexturePtr newTexture ) {
  if ( !channels[RE_MIXT_TEX1].isNull() && channels[RE_MIXT_TEX1]->getName() == textureName ) {
    channels[RE_MIXT_TEX1].clear();
    channels[RE_MIXT_TEX1] = newTexture;
  }  
  if ( !channels[RE_MIXT_TEX2].isNull() && channels[RE_MIXT_TEX2]->getName() == textureName ) {
    channels[RE_MIXT_TEX2].clear();
    channels[RE_MIXT_TEX2] = newTexture;
  }  
  if ( !channels[RE_MIXT_MIXER].isNull() && channels[RE_MIXT_MIXER]->getName() == textureName ) {
    channels[RE_MIXT_MIXER].clear();
    channels[RE_MIXT_MIXER] = newTexture;
  }
}
 
void ReMixTexture::serialize( QDataStream& dataStream ) {
  ReTexture::serialize(dataStream);
  dataStream << mixAmount;
  serializeChannels(dataStream);
};

void ReMixTexture::deserialize( QDataStream& dataStream ) {
  ReTexture::deserialize(dataStream);
  dataStream >> mixAmount;
  deserializeChannels(dataStream, parent);
};

QString ReMixTexture::toString() {
  return QString("Mix texture: %1, %2")
           .arg(channels[RE_MIXT_TEX1]->getName())
           .arg(channels[RE_MIXT_TEX2]->getName());
};

} // namespace
