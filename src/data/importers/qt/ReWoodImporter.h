/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_WOOD_IMPORTER_H
#define QT_WOOD_IMPORTER_H

#include <QVariantMap>

#include "ReProceduralNoiseImporter.h"
#include "textures/ReWood.h"


namespace Reality {

/*
  Class: ReQtWoodExporter
 */

class ReQtWoodImporter : public ReQtProceduralNoiseImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReWoodPtr tex = baseTex.staticCast<ReWood>();
    ReQtProceduralNoiseImporter::restoreTexture(tex, data);

    tex->setTurbulence(data.value("turbulence").toFloat());
    tex->setVeinWave(static_cast<ReWood::VeinWave>
      (data.value("veinWave").toInt())
    );
    tex->setWoodPattern(static_cast<ReWood::WoodPattern>
      (data.value("woodPattern").toInt())
    );
    tex->setSoftNoise(data.value("softNoise").toBool());
  };
};

}
#endif
