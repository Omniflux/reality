/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_PROCEDURALNOISE_IMPORTER_H
#define QT_PROCEDURALNOISE_IMPORTER_H

#include "textures/ReProceduralNoise.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/*
  Class: ReQtProceduralnoiseExporter
 */

class ReQtProceduralNoiseImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReProceduralNoisePtr tex = baseTex.staticCast<ReProceduralNoise>();
    restoreBasicProperties(tex, data);

    tex->setNoiseSize(data.value("noiseSize").toFloat());
    tex->setBrightness(data.value("brightness").toFloat());
    tex->setContrast(data.value("contrast").toFloat());
    tex->setNoiseBasis(static_cast<ReProceduralNoise::NoiseDistortionType>
      (data.value("noiseBasis").toInt())
    );
    restore3DMapping(tex, data);
  };
};

}
#endif
