/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CONSTANT_EXPORTER_H
#define QT_CONSTANT_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureExporter.h"
#include "ReTexture.h"
#include "textures/ReConstant.h"


namespace Reality {

/**
 * Exports the Color constant texture to the QVariantMap format
 */
class REALITY_LIB_EXPORT ReQtConstantExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
   auto tex = static_cast<const ReConstant*>(baseTex);
    storeBasicProperties(tex);
    texData["color"] = tex->getColor().name();
    return texData;
  };
};

}
#endif
