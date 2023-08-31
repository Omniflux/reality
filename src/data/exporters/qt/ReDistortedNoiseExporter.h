/**
  \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_DISTORTEDNOISE_EXPORTER_H
#define QT_DISTORTEDNOISE_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReProceduralNoiseExporter.h"
#include "ReQtTextureExporter.h"
#include "textures/ReDistortedNoise.h"


namespace Reality {

/**
 * Exports the Distorted Noise texture to the QVariantMap format
 */
class REALITY_LIB_EXPORT ReQtDistortedNoiseExporter : public ReQtProceduralNoisesExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
   auto tex = static_cast<const ReDistortedNoise*>(baseTex);

    ReQtProceduralNoisesExporter::exportTexture(tex);
    texData["distortionType"]   = tex->getDistortionType();
    texData["distortionAmount"] = tex->getDistortionAmount();
    return texData;    
  };
};

}
#endif
