/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReBricks.h"

#include "ReTextureContainer.h"
#include "textures/ReConstant.h"


namespace Reality {

// Static variables definition
QString ReBricks::lutTypesToStrings[ReBricks::CHAINLINK+1] = {
  QString("Stacked"),
  QString("Flemish"),
  QString("English"),
  QString("Herringbone"),
  QString("Basket"),
  QString("Chain link")
};


ReBricks::ReBricks( const QString name,
                ReTextureContainer* parentMat,
                const ReTextureDataType dataType ) : 
  ReTexture3D(name, parentMat),
  // Dimensions from wikipedia 
  width(0.228f),
  height(0.108f),
  depth(0.054f),
  bevel(0.005f),
  offset(0.114f),
  mortarSize(0.008f),
  brickType(STACKED)
{
  type = TexBricks;
  textureDataType = dataType;
  initChannels();
};

// Conversion ctor
ReBricks::ReBricks( const ReTexturePtr srcTex ) :
  ReTexture3D(srcTex),
  width(0.228f),
  height(0.108f),
  depth(0.054f),
  bevel(0.005f),
  offset(0.114f),
  mortarSize(0.008f),
  brickType(STACKED)
{
  textureDataType = srcTex->getDataType();
  type = TexBricks;

  if (srcTex->getType() == TexBricks) {
    ReBricksPtr b2 = srcTex.staticCast<ReBricks>();
    width        = b2->width;
    height       = b2->height;
    depth        = b2->depth;
    bevel        = b2->bevel;
    offset       = b2->offset;
    mortarSize   = b2->mortarSize;
    brickType    = b2->brickType;    
  }
  initChannels();
}

void ReBricks::initChannels() {
  QString brickName  = QString("%1_brickColor").arg(name);
  QString mortarName = QString("%1_brickMortar").arg(name);
  QString modName    = QString("%1_brickMod").arg(name);

  if (parent) {
    // If the textures exist already, as in the case of serializing/
    // deserializing, then we simply connect to them
    auto brickTex = parent->getTexture(brickName);
    if (!brickTex.isNull()) {
      channels[RE_BRICK_BRICK_CHANNEL] = brickTex;
    }
    auto mortarTex = parent->getTexture(mortarName);
    if (!mortarTex.isNull()) {
      channels[RE_BRICK_MORTAR_CHANNEL] = mortarTex;
    }
    auto modTex = parent->getTexture(modName);
    if (!modTex.isNull()) {
      channels[RE_BRICK_MODULATION_CHANNEL] = modTex;
    }
    if ( !brickTex.isNull() && !mortarTex.isNull() && !modTex.isNull()) {
      // we're done here
      return;
    }    
  }
  // Brick texture
  if ( textureDataType == color ) {
    channels[RE_BRICK_BRICK_CHANNEL] = ReTexturePtr(
                          new ReConstant(QString("%1_brickColor").arg(name), parent, QColor(130, 82, 82) ) 
                        );
    // Mortar texture
    channels[RE_BRICK_MORTAR_CHANNEL] = ReTexturePtr(
                           new ReConstant(QString("%1_brickMortar").arg(name), parent, QColor(75, 75, 75) ) 
                         );
    // Brick modulation texture
    channels[RE_BRICK_MODULATION_CHANNEL] = ReTexturePtr(
                           new ReConstant(
                                 QString("%1_brickMod").arg(name),
                                 parent, QColor(127, 127, 127) 
                               ) 
                      );
  }
  else {
    channels[RE_BRICK_BRICK_CHANNEL] = ReTexturePtr(
                          new ReConstant(QString("%1_brickColor").arg(name), parent, 255 ) 
                        );
    // Mortar texture
    channels[RE_BRICK_MORTAR_CHANNEL] = ReTexturePtr(
                           new ReConstant(QString("%1_brickMortar").arg(name), parent, 5) 
                         );
    channels[RE_BRICK_MODULATION_CHANNEL] = ReTexturePtr(
                           new ReConstant(QString("%1_brickMod").arg(name), parent, 229) 
                      );
  }
  parent->addTextureToCatalog(channels[RE_BRICK_BRICK_CHANNEL]);
  parent->addTextureToCatalog(channels[RE_BRICK_MORTAR_CHANNEL]);
  parent->addTextureToCatalog(channels[RE_BRICK_MODULATION_CHANNEL]);

}

ReBricks::~ReBricks() {
};


void ReBricks::setBrickTexture( ReTexturePtr newVal ) {
  channels[RE_BRICK_BRICK_CHANNEL] = newVal;
  if (!newVal.isNull() && parent) {
    newVal->reparent(parent);
    parent->addTextureToCatalog(newVal);
  }
};

  //! Sets the texture for the mortar 
void ReBricks::setMortarTexture( ReTexturePtr newVal ) {
  channels[RE_BRICK_MORTAR_CHANNEL] = newVal;
  if (!newVal.isNull() && parent) {
    newVal->reparent(parent);
    parent->addTextureToCatalog(newVal);
  }
};

//! Sets the texture for the mortar 
void ReBricks::setBrickModulationTexture( ReTexturePtr newVal ){
  channels[RE_BRICK_MODULATION_CHANNEL] = newVal;
  if (!newVal.isNull() && parent) {
    newVal->reparent(parent);
    parent->addTextureToCatalog(newVal);
  }
};

const ReTexturePtr ReBricks::getBrickTexture() const {
  return channels[RE_BRICK_BRICK_CHANNEL];
};

const ReTexturePtr ReBricks::getBrickModulationTexture() const {
  return channels[RE_BRICK_MODULATION_CHANNEL];
};

const ReTexturePtr ReBricks::getMortarTexture() const {
  return channels[RE_BRICK_MORTAR_CHANNEL];
};

QStringList ReBricks::getDependencies() {
  deps.clear();
  ReNodeDictionaryIterator i(channels);
  while( i.hasNext() ) {
    i.next();

    ReTexturePtr tex = i.value();
    if ( !tex.isNull() ) {
      deps << tex->getName();
    }
  }
  return deps;
};

QString ReBricks::getBrickTypeAsString() {
  switch( brickType ) {
    case STACKED: 
      return "stacked";
    case FLEMISH:
      return "flemish";
    case ENGLISH:
      return "english";
    case HERRINBONE:
      return "herringbone";
    case BASKET:
      return "basket";
    case CHAINLINK:
      return "chain link";
  }
  return "stacked";
}

const ReBricks::BrickType ReBricks::getBrickType() const {
  return brickType;
};

void ReBricks::setBrickType( BrickType newVal ) {
  brickType = newVal;
};

const float ReBricks::getWidth() const {
  return width;
};

void ReBricks::setWidth( float newVal ) {
  width = newVal;
};

const float ReBricks::getHeight() const {
  return height;
};

void ReBricks::setHeight( float newVal ) {
  height = newVal;
};

const float ReBricks::getDepth() const {
  return depth;
};

void ReBricks::setDepth( float newVal ) {
  depth = newVal;
};

const float ReBricks::getBevel() const {
  return bevel;
};

void ReBricks::setBevel( float newVal ) {
  bevel = newVal;
};

const float ReBricks::getOffset() const {
  return offset;
};

void ReBricks::setOffset( float newVal ) {
  offset = newVal;
};

const float ReBricks::getMortarSize() const {
  return mortarSize;
};

void ReBricks::setMortarSize( float newVal ) {
  mortarSize = newVal;
};

QString ReBricks::toString() {
  return "ReBricks texture";
};

void ReBricks::setNamedValue( const QString& name , const QVariant& value ) {
  if (name == "width") {
    width = value.toFloat();
  }
  else if (name == "height") {
    height = value.toFloat();
  }
  else if (name == "depth") {
    depth = value.toFloat();
  }
  else if (name == "bevel") {
    bevel = value.toFloat();
  }
  else if (name == "offset") {
    offset = value.toFloat();
  }
  else if (name == "mortarSize") {
    mortarSize = value.toFloat();
  }
  else if (name == "brickType") {
    brickType = static_cast<BrickType>(value.toInt());
  }
  else if (name == "brickTexture") {
    setBrickTexture(value.value<ReTexturePtr>());
  }
  else if (name == "modTexture") {
    setBrickModulationTexture(value.value<ReTexturePtr>());
  }
  else if (name == "mortarTexture") {
    setMortarTexture(value.value<ReTexturePtr>());
  }
  else {
    ReTexture3D::setNamedValue(name, value);
  }
};

const QVariant ReBricks::getNamedValue( const QString& name ) {
  QVariant val;
  if (name == "width") {
    return width;
  }
  else if (name == "height") {
    return height;
  }
  else if (name == "depth") {
    return depth;
  }
  else if (name == "bevel") {
    return bevel;
  }
  else if (name == "offset") {
    return offset;
  }
  else if (name == "mortarSize") {
    return mortarSize;
  }
  else if (name == "brickType") {
    return brickType;
  }
  else if (name == "brickTexture") {
    val.setValue(channels[RE_BRICK_BRICK_CHANNEL]);
  }
  else if (name == "modTexture") {
    val.setValue(channels[RE_BRICK_MODULATION_CHANNEL]);
  }
  else if (name == "mortarTexture") {
    val.setValue(channels[RE_BRICK_MORTAR_CHANNEL]);
  }
  else {
    return ReTexture3D::getNamedValue(name);
  }
  return val;
};

void ReBricks::replaceInnerTexture( const QString textureName, ReTexturePtr newTexture ) {
  ReNodeDictionaryIterator i(channels);
  QString key = ""; // Flag that the value has not been found
  while( i.hasNext() ) {
    i.next();
    if ( i.value()->getName() == textureName ) {
      key = i.key();
      break;
    }
  }
  if (key != "") {
    channels[key].clear();
    channels[key] = newTexture;
  }
}

void ReBricks::serialize( QDataStream& dataStream ) {
  ReTexture3D::serialize(dataStream);
  dataStream << (quint16) brickType << width 
             << height << depth 
             << bevel << offset 
             << mortarSize;
  serializeChannels(dataStream);
};


void ReBricks::deserialize( QDataStream& dataStream ) {
  ReTexture3D::deserialize(dataStream);
  quint16 brickTypeInt;  
  dataStream >> brickTypeInt >> width 
             >> height >> depth 
             >> bevel >> offset 
             >> mortarSize;

  brickType = static_cast<BrickType>(brickTypeInt);
  deserializeChannels(dataStream, parent);
};


} // namespace
