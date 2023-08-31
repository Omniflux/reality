/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_BRICKS_TEXTURE_EXPORTER_H
#define LUX_BRICKS_TEXTURE_EXPORTER_H

#include <QString>

#include "reality_lib_export.h"
#include "ReTexture.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "textures/ReBricks.h"


namespace Reality {

/**
  Class: ReLuxBricksExporter
 */
class REALITY_LIB_EXPORT ReLuxBricksExporter : public ReLuxTextureExporter {

public:


  const QString exportTexture( ReTexturePtr texBase, 
                               const QString& assignedName = "",
                               bool isForPreview = false ) {
    ReBricksPtr tex = texBase.staticCast<ReBricks>();
    QString mapType = getMappingAsString(tex);
    QString mapScale = QString("%1 %1 %1").arg((double)tex->getScale(),0,'f',4);
    if (isForPreview) {
      mapType = "global";
      // We double the scale to make it more visible in preview
      mapScale = QString("%1 %1 %1").arg((double)tex->getScale() * 2,0,'f',4);
    }

    return QString(                   
             "Texture \"%1\" \"%2\" \"brick\" \"string brickbond\" [\"%3\"]\n"
             "  \"float brickwidth\" [%4]\n"
             "  \"float brickheight\" [%5]\n"
             "  \"float brickdepth\" [%6]\n"
             "  \"float brickbevel\" [%7]\n"
             "  \"float brickrun\" [%8]\n"
             "  \"float mortarsize\" [%9]\n"
             "  \"texture bricktex\" [\"%10\"]\n"
             "  \"texture mortartex\" [\"%11\"]\n"
             "  \"texture brickmodtex\" [\"%12\"]\n"
             "  \"string coordinates\" [\"%13\"]\n"
             "  \"vector translate\" [0.0 0.0 0.0]\n"
             "  \"vector rotate\" [0.0 0.0 0.0]\n"
             "  \"vector scale\" [%14]"
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
           .arg(tex->getDataTypeAsString())
           .arg(tex->getBrickTypeAsString())
           .arg(tex->getNamedValue("width").toFloat())
           .arg(tex->getNamedValue("height").toFloat())
           .arg(tex->getNamedValue("depth").toFloat())
           .arg(tex->getNamedValue("bevel").toFloat())
           .arg(tex->getNamedValue("offset").toFloat())
           .arg(tex->getNamedValue("mortarSize").toFloat())
           .arg(tex->getBrickTexture()->getUniqueName())
           .arg(tex->getMortarTexture()->getUniqueName())
           .arg(tex->getBrickModulationTexture()->getUniqueName())
           .arg(mapType)
           .arg(mapScale)
           ;
  };
};

}
#endif
