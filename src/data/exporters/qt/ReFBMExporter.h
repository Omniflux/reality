/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_FBM_EXPORTER_H
#define QT_FBM_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReTexture.h"
#include "ReQtTextureExporter.h"
#include "textures/ReFBM.h"


namespace Reality {

/**
 * Exports the FMB texture to the QVariantMap format
 */
class REALITY_LIB_EXPORT ReQtFBMExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    auto tex = static_cast<const ReFBM*>(baseTex);
    storeBasicProperties(tex);

    texData["octaves"]   = tex->getOctaves();
    texData["roughness"] = tex->getRoughness();
    texData["wrinkled"]  = tex->isWrinkled();
    export3DMapping(tex);
    
    return texData;
  };
};

}
#endif
