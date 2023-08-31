/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_COLORMATH_EXPORTER_H
#define QT_COLORMATH_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureExporter.h"
#include "ReTexture.h"
#include "textures/ReColorMath.h"


namespace Reality {

/**
 * Exports the Color Math texture to the QVariantMap format 
 */
class REALITY_LIB_EXPORT ReQtColorMathExporter : public ReQtTextureExporter {

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
