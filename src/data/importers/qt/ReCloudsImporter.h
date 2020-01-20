/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CLOUDS_IMPORTER_H
#define QT_CLOUDS_IMPORTER_H

#include "textures/ReClouds.h"
#include "ReProceduralNoiseImporter.h"

namespace Reality {

/*
  Class: ReQtCloudsExporter
 */

class RE_LIB_ACCESS ReQtCloudsImporter : public ReQtProceduralNoiseImporter {

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
