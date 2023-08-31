/**
  \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef JSON_BRICK_TEXTURE_EXPORTER_H
#define JSON_BRICK_TEXTURE_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureExporter.h"
#include "ReTexture.h"
#include "textures/ReBricks.h"


namespace Reality {

/**
 * Exports the Brick texture to the QVariantMap format
 */

class REALITY_LIB_EXPORT ReQtBricksExporter : public ReQtTextureExporter {

public:
  QVariantMap exportTexture( const ReTexture* baseTex ) {
    const Bricks* tex = static_cast<const Bricks*>(baseTex);
    storeBasicProperties(tex);

    texData["brickType"]  = tex->getBrickType();
    texData["width"]      = tex->getWidth();
    texData["height"]     = tex->getHeight();
    texData["depth"]      = tex->getDepth();
    texData["bevel"]      = tex->getBevel();
    texData["offset"]     = tex->getOffset();
    texData["mortarSize"] = tex->getMortarSize();
    exportChannels(tex);
    export3DMapping(tex);

    return texData;    
  };
};

}
#endif
