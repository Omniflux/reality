/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReBand.h"
#include "ReConstant.h"
#include "ReTextureCreator.h"

namespace Reality {

// Table of colors to initialize the entries in the texture
// The colors are the same used by Poser 9/2012 
QString BT_colorTable[REBT_DEFAULT_NUM_ENTRIES] = {
  QString("#953400"),
  QString("#FB7207"),
  QString("#FED727"),
  QString("#FEF8ED")
};

ReBand::ReBand( const QString name, ReTextureContainer* parentMat ) : 
  ReTexture(name, parentMat),
  numBands(REBT_DEFAULT_NUM_ENTRIES),
  amount(1.0)
{
  type = TexBand;
  initChannels(REBT_DEFAULT_NUM_ENTRIES);
}

// texture conversion ctor
ReBand::ReBand( const ReTexturePtr srcTex ) :
  ReTexture(srcTex),
  numBands(REBT_DEFAULT_NUM_ENTRIES),
  amount(1.0)
{
  type = TexBand;
  initChannels(REBT_DEFAULT_NUM_ENTRIES);
}

ReBand::~ReBand() {
}

void ReBand::setNumBands( int newVal ) {
  numBands = newVal;
}


void ReBand::initChannels( const qint16 numEntries ) {
  if (parent) {
    // If the textures exist already, as in the case of serializing/
    // deserializing, then we simply connect to them
    int textureFound = 0;
    for (int i = 0; i < numEntries; i++) {      
      QString channelName = QString("tex%1").arg(i+1);
      QString texName = QString("%1_tex%2").arg(name).arg(i+1);

      auto tex = parent->getTexture(texName);
      if (!tex.isNull()) {
        channels[channelName] = tex;
        textureFound++;
      }
    }
    if ( textureFound == numEntries ) {
      // we're done here
      return;
    }    
  }

  for (int i = 0; i < numEntries; i++) {
    QString channelName = QString("tex%1").arg(i+1);
    channels[channelName] = ReTexturePtr(
                             new ReConstant(
                               QString("%1_tex%2").arg(name).arg(i+1),
                               parent,
                               BT_colorTable[i]
                             )
                           ); 
    // Space the bands evenly
    bandOffsets[channelName] = 1.0 / (numEntries-1) * i;
    if (parent) {
      parent->addTextureToCatalog(channels.value(channelName));      
    }
  }
  // The texture that does the actual mapping of each input texture to the
  // final result. This is a float texture.
  channels["map"] = ReTexturePtr();
  numBands = numEntries;
}

void ReBand::reparent( ReTextureContainer* parentMat ) {
  ReTexture::reparent(parentMat);
  ReNodeDictionaryIterator i(channels);
  while( i.hasNext() ) {
    i.next();
    ReTexturePtr tex = i.value();
    if (tex.isNull()) {
      continue;
    }
    if (parent) {
      parent->addTextureToCatalog(tex);
    }
    tex->reparent(parentMat);
  }
}

QStringList ReBand::getDependencies() {
  deps.clear();
  ReNodeDictionaryIterator i(channels);
  while( i.hasNext() ) {
    i.next();
    ReTexturePtr tex = i.value();
    if (tex.isNull()) {
      continue;
    }
    deps << tex->getName();
  }

  return deps;
};

void ReBand::replaceInnerTexture( const QString textureName, ReTexturePtr newTexture ) {
  ReNodeDictionaryIterator i(channels);
  QString key = ""; // Flag that the value has not been found
  while( i.hasNext() ) {
    i.next();
    ReTexturePtr tex = i.value();
    if (tex.isNull()) {
      continue;
    }
    if ( tex->getName() == textureName ) {
      key = i.key();
      break;
    }
  }
  if (key != "") {
    channels[key].clear();
    channels[key] = newTexture;
  }
}

float ReBand::getAmount() const {
  return amount;
};

void ReBand::setAmount( float newVal ) {
  amount = newVal;
};

int ReBand::getNumBands() const {
  return numBands;
};

ReBandOffsets ReBand::getOffsets() const {
  return bandOffsets;
}

void ReBand::setNamedValue( const QString& vname , const QVariant& value ) {
  // We set the value for each texture with a vname value of 
  // "tex1", "tex2" ... "texN". We do the same for the offset: "offset1"..."offsetN"
  QRegExp textureRE("tex\\d");
  QRegExp offsetRE("offset(\\d)");
  if (textureRE.indexIn(vname) != -1) {
    channels[vname] = value.value<ReTexturePtr>();
  }
  else if (offsetRE.indexIn(vname) != -1) {
    int texNum = offsetRE.cap(1).toInt();
    bandOffsets[QString("tex%1").arg(texNum)] = value.toFloat();
  }
  else if (vname == "map") {
    channels["map"] = value.value<ReTexturePtr>();
  }
  else if (vname == "amount") {
    amount = value.toFloat();
  }
  else {
    ReTexture::setNamedValue(vname, value);
  }
}

const QVariant ReBand::getNamedValue( const QString& vname ) {
  QVariant val;

  QRegExp textureRE("tex\\d");
  QRegExp offsetRE("offset(\\d)");
  if (textureRE.indexIn(vname) != -1) {
    val.setValue(channels[vname]);
  }
  else if (offsetRE.indexIn(vname) != -1) {
    int texNum = offsetRE.cap(1).toInt();
    val.setValue(bandOffsets[ QString("tex%1").arg(texNum) ]);
  }
  else if (vname == "map") {
    val.setValue(channels["map"]);
  }
  else if (vname == "amount") {
    val = amount ;
  }
  else if (vname == "count") {
    return numBands;
  }
  else {
    return ReTexture::getNamedValue(vname);
  }
  return val;
}

void ReBand::serialize( QDataStream& dataStream ) {
  ReTexture::serialize(dataStream);
  dataStream << amount << bandOffsets;
  serializeChannels(dataStream);
};


void ReBand::deserialize( QDataStream& dataStream ) {
  ReTexture::deserialize(dataStream);
  dataStream >> amount >> bandOffsets;

  deserializeChannels(dataStream, parent);  
};

QString ReBand::toString() {
  return "ReBand texture";
};


}
