/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_MIXTEXTURE_IMPORTER_H
#define QT_MIXTEXTURE_IMPORTER_H

#include "textures/ReMix.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/*
  Class: ReQtColorMathExporter
 */

class ReQtMixTextureImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReMixTexturePtr tex = baseTex.staticCast<ReMixTexture>();
    restoreBasicProperties(tex, data);

    tex->setMixAmount(data.value("mixAmount").toFloat());
    restoreChannels(tex, data);
  };
};

}
#endif
