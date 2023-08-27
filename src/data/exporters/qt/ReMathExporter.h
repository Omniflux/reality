/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef QT_MATH_EXPORTER_H
#define QT_MATH_EXPORTER_H

#include "textures/ReMath.h"
#include "ReQtTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

class REALITY_LIB_EXPORT ReQtMathExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReMath*>(baseTex);
    storeBasicProperties(tex);

    texData["amount1"]  = tex->getAmount1();
    texData["amount2"]  = tex->getAmount2();
    texData["function"] = tex->getFunction();
    // Export the textures/channels
    exportChannels(tex);
    return texData;
  };
};

}
#endif
