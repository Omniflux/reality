/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef TQ_MIX_TEXTURE_EXPORTER_H
#define TQ_MIX_TEXTURE_EXPORTER_H

#include "textures/ReMix.h"
#include "ReQtTextureExporter.h"

namespace Reality {

class RE_LIB_ACCESS ReQtMixTextureExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReMixTexture*>(baseTex);
    storeBasicProperties(tex);

    texData["mixAmount"]  = tex->getMixAmount();
    exportChannels(tex);
    return texData;
  };
};

}
#endif
