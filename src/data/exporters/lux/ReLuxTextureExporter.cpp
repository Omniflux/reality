/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLuxTextureExporter.h"

#include "ReLuxTextureExporterFactory.h"
#include "ReTexture.h"
#include "textures/ReImageMap.h"


namespace Reality {

// Static variable definition
QString ReLuxTextureExporter::tmpStr;
ReTextureCache ReLuxTextureExporter::textureCache;
bool ReLuxTextureExporter::cachingEnabled = false;

ReTexturePtr ReLuxTextureExporter::writeFloatVersion( ReTexturePtr tex, QString& scene ) {
  if (tex->getType() != TexImageMap) {
    return tex;
  }
  QString textureName = getFloatTextureName(tex);
  ImageMapPtr floatTex = ImageMapPtr( new ImageMap(*tex.staticCast<ImageMap>()) );
  // Set it to be of type float...
  floatTex->setTextureDataType(ReTexture::numeric);

  // .. and set a new name to avoid conflicts with the original texture       
  floatTex->setName(textureName);

  // Get the full name to reference it in other textures...
  // QString texFullName = floatTex->getUniqueName();

  // Obtain the exporter...
  if (!isTextureInCache(floatTex)) {
    ReLuxTextureExporterPtr imExporter = ReLuxTextureExporterFactory::getExporter(floatTex);
    // ... and export it!
    scene += imExporter->exportTexture(floatTex);
    addTextureToCache(floatTex);
  }
  else {
    return getTextureFromCache(floatTex);
  }

  return floatTex;
}

void ReLuxTextureExporter::initializeTextureCache() {
  textureCache.clear();
}

void ReLuxTextureExporter::enableTextureCache( bool yesNo ) {
  cachingEnabled = yesNo;
}

bool ReLuxTextureExporter::isTextureInCache( ReTexturePtr tex ) {
  if (!cachingEnabled) {
    return false;
  }
  return textureCache.contains(tex->getGUID());
}

bool ReLuxTextureExporter::isTextureInCache( const QString& key ) {
  if (!cachingEnabled) {
    return false;
  }
  return textureCache.contains(key);
}

ReTexturePtr ReLuxTextureExporter::getTextureFromCache( ReTexturePtr tex ) {
  if (tex.isNull()) {
    return tex;
  }
  // If caching is not enabled then the same texture of the input is
  // returned. This voids the impact of the cache. In case any issue
  // needs to be tested, we can simply disable the cache and all calls
  // to it will simply return the original textures
  if (cachingEnabled) {
    QString key = tex->getGUID();
    auto cTex = textureCache.value(key);
    if (!cTex.isNull()) {
      return cTex;
    }
    else if (!textureCache.contains(key)){
      textureCache[key] = tex;
    }
  }
  return tex;
}

ReTexturePtr ReLuxTextureExporter::getTextureFromCache( const QString& texGUID ) {
  if (cachingEnabled) {
    return textureCache.value(texGUID);
  }
  return ReTexturePtr();
}

void ReLuxTextureExporter::addTextureToCache( ReTexturePtr tex ) {
  if (!cachingEnabled) {
    return;
  }
  QString id = tex->getGUID();
  if (!textureCache.contains(id)) {
    textureCache[id] = tex;
  }  
}

} // namespace

