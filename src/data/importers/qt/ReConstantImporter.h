/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CONSTANT_IMPORTER_H
#define QT_CONSTANT_IMPORTER_H

#include "textures/ReConstant.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/*
  Class: ReQtConstantExporter
 */

class RE_LIB_ACCESS ReQtConstantImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReConstantPtr tex = baseTex.staticCast<ReConstant>();
    restoreBasicProperties(tex, data);

    tex->setColor(data.value("color").toString());
  };
};

}
#endif
