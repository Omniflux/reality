/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_BAND_IMPORTER_H
#define QT_BAND_IMPORTER_H

#include "textures/ReBand.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/**
  Class: ReQtBandExporter
 */

class RE_LIB_ACCESS ReQtBandImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReBandPtr tex = baseTex.staticCast<ReBand>();
    restoreBasicProperties(tex, data);

    tex->setNumBands(data.value("numBands").toInt());
    QMapIterator<QString, QVariant> bi(data.value("offsets").toMap());
    int i = 1;
    while( bi.hasNext() ) {
      bi.next();
      tex->setNamedValue(QString("offset%1").arg(i), bi.value());
      i++;
    }
    tex->setAmount(data.value("amount").toFloat());
    restoreChannels(tex, data);
  };
};

}
#endif
