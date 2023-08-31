/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CHECKERS_TEXTURE_EXPORTER_H
#define QT_CHECKERS_TEXTURE_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureExporter.h"
#include "ReTexture.h"
#include "textures/ReCheckers.h"


namespace Reality {

/**
 * Exports the Checkers texture to the \ref QVariantMap format
 */

class REALITY_LIB_EXPORT ReQtCheckersExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    const ReCheckers* tex = static_cast<const ReCheckers*>(baseTex);
    storeBasicProperties(tex);

    // The JSON library doesn't handle floats, only doubles
    texData["is3D"]  = tex->is3D();
    exportChannels(tex);
    export3DMapping(tex);
    return texData;
  };
};

}
#endif
