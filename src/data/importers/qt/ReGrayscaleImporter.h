/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_GRAYSCALE_IMPORTER_H
#define QT_GRAYSCALE_IMPORTER_H

#include "textures/ReGrayscale.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/**
  Class: ReQtGrayscaleExporter
 */

class RE_LIB_ACCESS ReQtGrayscaleImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReGrayscalePtr tex = baseTex.staticCast<ReGrayscale>();
    restoreBasicProperties(tex, data);

    tex->setRgbChannel(static_cast<RGBChannel>(data.value("rgbChannel").toInt()));
    tex->setTexture(baseTex->getParent()->getTexture(data.value("texture").toString()));
    tex->setColor(QColor(data.value("color").toString()));
    restoreChannels(tex, data);
  };
};

}
#endif
