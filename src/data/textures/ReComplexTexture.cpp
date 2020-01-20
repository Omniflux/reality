/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReComplexTexture.h"
#include "ReTools.h"

namespace Reality {

ReTexturePtr ReComplexTexture::getChannel( const QString channelID ) const {
  return channels[channelID];
}

bool ReComplexTexture::hasChannel( const QString& channelID ) const {
  return channels.contains(channelID);
}

const ReNodeDictionary ReComplexTexture::getChannels() const {
  return channels;
}

void ReComplexTexture::setChannel( const QString channelName, ReTexturePtr texture ) {
  if ( channels.contains(channelName) ) {
    channels[channelName].clear();
  }
  if (!texture.isNull()) {
    channels[channelName] = texture;
    auto parentMat = texture->getParent();
    if (parentMat) {
      parentMat->addTextureToCatalog(texture);
    }
  }
};

void ReComplexTexture::deleteChannelTexture( const QString& channelName ) {
  if ( channels.contains(channelName) ) {
    auto mat = channels[channelName]->getParent();
    QString texName = channels[channelName]->getName();
    channels[channelName].clear();
    // Try to delete the texture if it's not used by anybody else
    if ( mat->textureIsUnused(texName) ) {
      mat->deleteTexture(texName);
    }
  }
}

const QString ReComplexTexture::identifyChannel( const ReTexturePtr tex ) const {
  if (tex.isNull()) {
    return "";
  }
  ReNodeDictionaryIterator i(channels);
  while( i.hasNext() ) {
    i.next();
    ReTexturePtr chTex = i.value();
    //!!! TODO: Check if we can simply test the pointers
    if (!chTex.isNull() && (chTex->getName() == tex->getName())) {
      return i.key();
    }
    else {
      auto subTex = chTex.dynamicCast<ReComplexTexture>();
      if (!subTex.isNull()) {
        QString channelName = subTex->identifyChannel(tex);
        if (!channelName.isEmpty()) {
          return QString("%1|%2").arg(i.key()).arg(channelName);
        }
      }
    }
  }
  return "";
}

void ReComplexTexture::serializeChannels( QDataStream& dataStream ) {
  // Number of channels
  dataStream << (quint16) channels.count();
  ReNodeDictionaryIterator ic(channels);  
  while(ic.hasNext()) {
    ic.next();
    
    ReTexturePtr tex = ic.value();

    // Name of the channel and a flag to declare if the texture exists
    bool texPresent = !tex.isNull();
    dataStream << ic.key() << texPresent;
    // Texture
    if (texPresent) {
      dataStream << tex->getName();   
    }
  }
};


void ReComplexTexture::deserializeChannels( QDataStream& dataStream, 
                                            ReTextureContainer* cont ) 
{
  quint16 numChannels;

  dataStream >> numChannels;
  for (int i = 0; i < numChannels; i++) {
    QString channelName;
    bool texPresent;
    dataStream >> channelName >> texPresent;
    if (texPresent) {
      QString texName;
      dataStream >> texName;

      if (channels.contains(texName) && !channels[texName].isNull()) {
        auto parent = channels.value(texName)->getParent();
        if (parent) {
          parent->deleteTexture(texName);
        }
      }
      channels[channelName] = cont->getTexture(texName);
    }
    else {
      channels[channelName] = ReTexturePtr();
    }
  }
};


} // namespace
