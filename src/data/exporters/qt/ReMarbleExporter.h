/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef QT_MARBLE_EXPORTER_H
#define QT_MARBLE_EXPORTER_H

#include "ReProceduralNoiseExporter.h"
#include "textures/ReMarble.h"
#include "ReQtTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {


class REALITY_LIB_EXPORT ReQtMarbleExporter : public ReQtProceduralNoisesExporter  {

public:
  virtual QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReMarble*>(baseTex);
    ReQtProceduralNoisesExporter::exportTexture(tex);

    texData["depth"]       = tex->getDepth();
    texData["turbulence"]  = tex->getTurbulence();
    texData["veinQuality"] = tex->getVeinQuality();
    texData["veinWave"]    = tex->getVeinWave();
    texData["softNoise"]   = tex->hasSoftNoise();
    export3DMapping(tex);
    return texData;    
  };
};

}
#endif
