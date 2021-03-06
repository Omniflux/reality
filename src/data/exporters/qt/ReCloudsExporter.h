/**
  \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CLOUDS_EXPORTER_H
#define QT_CLOUDS_EXPORTER_H

#include "textures/ReClouds.h"
#include "ReProceduralNoiseExporter.h"
#include <QVariantMap>

namespace Reality {

/**
 * Exports the Clouds texture to the QVariantMap format
 */

class RE_LIB_ACCESS ReQtCloudsExporter : public ReQtProceduralNoisesExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const Clouds*>(baseTex);
    ReQtProceduralNoisesExporter::exportTexture(tex);

    texData["usesHardNoise"] = tex->usesHardNoise();
    texData["noiseDepth"]    = tex->getNoiseDepth();
    export3DMapping(tex);
    return texData;
  };
};

}
#endif
