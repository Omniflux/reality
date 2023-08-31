/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_GRAYSCALE_IMPORTER_H
#define QT_GRAYSCALE_IMPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureImporter.h"
#include "textures/ReGrayscale.h"


namespace Reality {

/**
  Class: ReQtGrayscaleExporter
 */

class REALITY_LIB_EXPORT ReQtGrayscaleImporter : public ReQtTextureImporter {

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
