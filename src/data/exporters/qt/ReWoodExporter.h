/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_WOOD_EXPORTER_H
#define QT_WOOD_EXPORTER_H

#include "ReProceduralNoiseExporter.h"
#include "textures/ReWood.h"
#include "ReQtTextureExporter.h"

namespace Reality {

class RE_LIB_ACCESS ReQtWoodExporter : public ReQtProceduralNoisesExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReWood*>(baseTex);
    ReQtProceduralNoisesExporter::exportTexture(tex);

    texData["turbulence"]  = tex->getTurbulence();
    texData["veinWave"]    = tex->getVeinWave();
    texData["woodPattern"] = tex->getWoodPattern();
    texData["softNoise"]   = tex->hasSoftNoise();
    export3DMapping(tex);
    return texData;    
  };
};

}
#endif
