/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_FBM_IMPORTER_H
#define QT_FBM_IMPORTER_H

#include "textures/ReFBM.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/*
  Class: ReQtFBMExporter
 */

class RE_LIB_ACCESS ReQtFBMImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReFBMPtr tex = baseTex.staticCast<ReFBM>();
    restoreBasicProperties(tex, data);

    tex->setOctaves(data.value("octaves").toInt());
    tex->setRoughness(data.value("roughness").toFloat());
    tex->setWrinkled(data.value("wrinkled").toBool());
    restore3DMapping(tex,data);    
  };
};

}
#endif
