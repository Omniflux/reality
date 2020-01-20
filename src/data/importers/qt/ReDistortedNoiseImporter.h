/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_DISTORTEDNOISE_IMPORTER_H
#define QT_DISTORTEDNOISE_IMPORTER_H

#include "textures/ReDistortedNoise.h"
#include "ReQtTextureImporter.h"
#include "ReProceduralNoiseImporter.h"

namespace Reality {

/*
  Class: ReQtDistortednoiseExporter
 */

class RE_LIB_ACCESS ReQtDistortedNoiseImporter : public ReQtProceduralNoiseImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReDistortedNoisePtr tex = baseTex.staticCast<ReDistortedNoise>();
    ReQtProceduralNoiseImporter::restoreTexture(tex, data);

    tex->setDistortionType(static_cast<ReProceduralNoise::NoiseDistortionType>
      (data.value("distortionType").toInt())
    );
    tex->setDistortionAmount(data.value("distortionAmount").toFloat());
  };
};

}
#endif
