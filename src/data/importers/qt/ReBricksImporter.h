/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_BRICK_IMPORTER_H
#define QT_BRICK_IMPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureImporter.h"
#include "textures/ReBricks.h"


namespace Reality {

/**
 * Imports a Brick texture from data passed via a QVariantMap
 */
class REALITY_LIB_EXPORT ReQtBricksImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    BricksPtr tex = baseTex.staticCast<Bricks>();
    restoreBasicProperties(tex, data);

    tex->setBrickType(static_cast<Bricks::BrickType>(data.value("brickType").toInt()));
    tex->setWidth(data.value("width").toFloat());
    tex->setHeight(data.value("height").toFloat());
    tex->setDepth(data.value("depth").toFloat());
    tex->setBevel(data.value("bevel").toFloat());
    tex->setOffset(data.value("offset").toFloat());
    tex->setMortarSize(data.value("mortarSize").toFloat());
    restoreChannels(tex,data);
    restore3DMapping(tex,data);    
  };
};

}
#endif
