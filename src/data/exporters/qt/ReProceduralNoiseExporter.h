/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef QT_PROCEDURALNOISE_EXPORTER_H
#define QT_PROCEDURALNOISE_EXPORTER_H

#include "textures/ReProceduralNoise.h"
#include "ReQtTextureExporter.h"

namespace Reality {

/**
 * Exports the base data for procedural noise to the QVariantMap format
 */
class RE_LIB_ACCESS ReQtProceduralNoisesExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReProceduralNoise*>(baseTex);
    storeBasicProperties(tex);

    // The JSON library doesn't handle floats, only doubles
    texData["noiseSize"]  = tex->getNoiseSize();
    texData["brightness"] = tex->getBrightness();
    texData["contrast"]   = tex->getContrast();
    texData["noiseBasis"] = tex->getNoiseBasis();
    export3DMapping(tex);
    return texData;    
  };
};

}
#endif
