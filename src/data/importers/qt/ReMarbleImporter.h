/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_MARBLE_IMPORTER_H
#define QT_MARBLE_IMPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReProceduralNoiseImporter.h"
#include "textures/ReMarble.h"


namespace Reality {

/*
  Class: ReQtMarbleExporter
 */

class REALITY_LIB_EXPORT ReQtMarbleImporter : public ReQtProceduralNoiseImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReMarblePtr tex = baseTex.staticCast<ReMarble>();
    ReQtProceduralNoiseImporter::restoreTexture(tex, data);

    tex->setDepth(data.value("depth").toInt());
    tex->setTurbulence(data.value("turbulence").toFloat());
    tex->setVeinQuality(static_cast<ReMarble::VeinQuality>
      (data.value("veinQuality").toInt())
    );
    tex->setVeinWave(static_cast<ReMarble::VeinWave>
      (data.value("veinWave").toInt())
    );
    tex->setSoftNoise(data.value("softNoise").toBool());
    restore3DMapping(tex,data);    
  };
};

}
#endif
