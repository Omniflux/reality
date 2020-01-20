/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_COLORMATH_EXPORTER_H
#define QT_COLORMATH_EXPORTER_H

#include "textures/ReColorMath.h"
#include "ReQtTextureExporter.h"

namespace Reality {

/**
 * Exports the Color Math texture to the QVariantMap format 
 */
class RE_LIB_ACCESS ReQtColorMathExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReColorMath*>(baseTex);
    storeBasicProperties(tex);

    texData["color1"]   = tex->getColor1().name();
    texData["color2"]   = tex->getColor2().name();
    texData["function"] = tex->getFunction();
    exportChannels(tex);
    return texData;
  };
};

}
#endif
