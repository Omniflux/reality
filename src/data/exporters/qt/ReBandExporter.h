/**
  \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_BAND_TEXTURE_EXPORTER_H
#define QT_BAND_TEXTURE_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureExporter.h"
#include "ReTexture.h"
#include "textures/ReBand.h"


namespace Reality {

/**
 * Exports the Band texture to the QVariantMap format
 */

class REALITY_LIB_EXPORT ReQtBandExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    const ReBand* tex = static_cast<const ReBand*>(baseTex);
    storeBasicProperties(tex);

    QVariantMap offsets;
    ReBandOffsetsIterator i(tex->getOffsets());
    while( i.hasNext() ) {
      i.next();
      offsets[i.key()] = i.value();
    }

    texData["numBands"] = tex->getNumBands();
    texData["offsets"]  = offsets;
    texData["amount"]   = tex->getAmount();
    exportChannels(tex);
    return texData;
  };
};

}
#endif
