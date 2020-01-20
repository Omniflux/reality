/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_IMAGE_MAP_EXPORTER_H
#define QT_IMAGE_MAP_EXPORTER_H

#include "textures/ReImageMap.h"
#include "ReQtTextureExporter.h"
#include "ReTools.h"
 
namespace Reality {

/**
 * This class exports the Image Map texture to the QVariantMap format
 */
class RE_LIB_ACCESS ReQtImageMapExporter : public ReQtTextureExporter {
public:

  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ImageMap*>(baseTex);
    storeBasicProperties(tex);

    texData["fileName"]   = normalizeRuntimePath(tex->getFileName());
    texData["gain"]       = tex->getGain();
    texData["gamma"]      = tex->getGamma();
    texData["rgbChannel"] = tex->getRgbChannel();
    texData["normalMap"]  = tex->isNormalMap();
    export2DMapping(tex);
    return texData;
  };
};

}
#endif
