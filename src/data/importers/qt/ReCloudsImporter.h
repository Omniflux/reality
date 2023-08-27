/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CLOUDS_IMPORTER_H
#define QT_CLOUDS_IMPORTER_H

#include "textures/ReClouds.h"
#include "ReProceduralNoiseImporter.h"
#include "reality_lib_export.h"

namespace Reality {

/*
  Class: ReQtCloudsExporter
 */

class REALITY_LIB_EXPORT ReQtCloudsImporter : public ReQtProceduralNoiseImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    CloudsPtr tex = baseTex.staticCast<Clouds>();
    ReQtProceduralNoiseImporter::restoreTexture(tex, data);

    tex->setHardNoise(data.value("usesHardNoise").toBool());
    tex->setNoiseDepth(data.value("noiseDepth").toInt());
    restore3DMapping(tex,data);    
  };
};

}
#endif
