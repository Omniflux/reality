/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef QT_GRAYSCALE_TEXTURE_EXPORTER_H
#define QT_GRAYSCALE_TEXTURE_EXPORTER_H

#include "textures/ReGrayscale.h"
#include "ReQtTextureExporter.h"

namespace Reality {

/**
 * This class exports the Grayscale texture to the JSON format
 */
class RE_LIB_ACCESS ReQtGrayscaleExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReGrayscale*>(baseTex);
    storeBasicProperties(tex);

    texData["rgbChannel"] = tex->getRgbChannel();
    texData["texture"]    = tex->getTexture()->getName();
    texData["color"]      = tex->getColor().name();
    exportChannels(tex);
    return texData;    
  };
};

}
#endif
