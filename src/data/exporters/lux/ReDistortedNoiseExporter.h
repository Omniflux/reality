/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_DISTORTEDNOISE_TEXTURE_EXPORTER_H
#define LUX_DISTORTEDNOISE_TEXTURE_EXPORTER_H

#include "textures/ReDistortedNoise.h"
#include "ReLuxTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * Exports a Distorted Noise texture to Lux
 */
class REALITY_LIB_EXPORT ReLuxDistortedNoiseExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texbase, const QString& assignedName = "", bool isForPreview = false ) {
    ReDistortedNoisePtr tex = texbase.staticCast<ReDistortedNoise>();    
    QString mapType = getMappingAsString(tex);
    QString mapScale = QString("%1 %1 %1").arg((double)tex->getScale(),0,'f',4);
    if (isForPreview) {
      mapType = "global";
    }
    return QString(                   
             "Texture \"%1\" \"float\" \"blender_distortednoise\" "
             "\"string type\" [\"%2\"] \"string noisebasis\" [\"%3\"] "
             "\"float bright\" [%4] \"float contrast\" [%5] "
             "\"float distamount\" [%6] "
             "\"float noisesize\" [%7] \"float nabla\" [0.25]"
             "\"string coordinates\" [\"%8\"] "
             "\"vector translate\" [0.0 0.0 0.0] "
             "\"vector rotate\" [0.0 0.0 0.0] "
             "\"vector scale\" [%9] "
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName) // 1
           .arg(tex->getDistortionTypeAsString())
           .arg(tex->getNoiseBasisAsString())                             // 3
           .arg(tex->getBrightness())
           .arg(tex->getContrast())                                       // 5
           .arg(tex->getDistortionAmount())
           .arg(tex->getNoiseSize())                                      // 7 
           .arg(mapType)
           .arg(mapScale);

  };
};

}
#endif
