/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_GRAYSCALE_TEXTURE_EXPORTER_H
#define LUX_GRAYSCALE_TEXTURE_EXPORTER_H

#include "textures/ReGrayscale.h"
#include "ReLuxTextureExporter.h"
#include "exporters/lux/ReImageMapExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h" 
#include "reality_lib_export.h"

namespace Reality {

/*
  Class: ReLuxGrayscaleExporter
 */

class REALITY_LIB_EXPORT ReLuxGrayscaleExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texbase, 
                               const QString& assignedName = "", 
                               bool isForPreview = false ) 
  {
    ReGrayscalePtr tex = texbase.staticCast<ReGrayscale>();    
    QString str;
    ReTexture::ReTextureDataType dataType = tex->getDataType();
    ReTexturePtr innerTex = ReLuxTextureExporter::getTextureFromCache(
                              tex->getTexture()
                            );

    // Rare but it can happen
    if (innerTex.isNull()) {
      return "";
    }
    // This is a color texture
    if (dataType == ReTexture::color) {
      // Create a new ImageMap texture based on the original one
      QString baseTexName;      
      if (innerTex->getType() == TexImageMap) {
        ImageMapPtr bwTex = ImageMapPtr( new ImageMap(innerTex) );
        bwTex->setRgbChannel(tex->getRgbChannel());
        ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(bwTex);
        baseTexName = QString("%1_channel_%2").arg(tex->getUniqueName()).arg(tex->getRGBChannelAsString());
        str += exporter->exportTexture(bwTex, baseTexName);
      }
      else {
        baseTexName = innerTex->getUniqueName();
      }

      str += QString(
               "Texture \"%1\" \"%2\" \"mix\" \"color tex1\" [0 0 0] "
               "\"color tex2\" [1.0 1.0 1.0] \"texture amount\" [\"%3\"] "
             )
             .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  
             .arg(tex->getDataTypeAsString())
             .arg(baseTexName);
    }
    else {
      if (innerTex->getType() == TexImageMap) {
        str += QString(
                 "Texture \"%1\" \"float\" \"imagemap\" "
                 "\"string filename\" [\"%2\"] \"string channel\" [\"%3\"] "
               )
               .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  
#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
               .arg(innerTex->getNamedValue("fileName").toString().replace('\\', '/'))
#else
               .arg(innerTex->getNamedValue("fileName").toString())
#endif
               .arg(tex->getRGBChannelAsString());
      }
    }
    return str;
  }

};

}; // Namespace

#endif
