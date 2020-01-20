/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CONSTANT_EXPORTER_H
#define QT_CONSTANT_EXPORTER_H

#include "textures/ReConstant.h"
#include "ReQtTextureExporter.h"

namespace Reality {

/**
 * Exports the Color constant texture to the QVariantMap format
 */
class RE_LIB_ACCESS ReQtConstantExporter : public ReQtTextureExporter {

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
