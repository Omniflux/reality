/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_CLOUDS_TEXTURE_EXPORTER_H
#define LUX_CLOUDS_TEXTURE_EXPORTER_H

#include <QString>

#include "reality_lib_export.h"
#include "ReTexture.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "textures/ReClouds.h"


namespace Reality {

/*
  Class: ReLuxCloudsExporter
 */

class REALITY_LIB_EXPORT ReLuxCloudsExporter : public ReLuxTextureExporter{

public:
  const QString exportTexture( ReTexturePtr texbase, const QString& assignedName = "", bool isForPreview = false ) {
    ReCloudsPtr tex = texbase.staticCast<ReClouds>();

    QString mapType = getMappingAsString(tex);
    QString mapScale = QString("%1 %1 %1").arg((double)tex->getScale(),0,'f',4);
    if (isForPreview) {
      mapType = "global";
      // We double the scale to make it more visible in preview
      mapScale = QString("%1 %1 %1").arg((double)tex->getScale() * 2,0,'f',4);
    }
    return QString(                   
             "Texture \"%1\" \"float\" \"blender_clouds\"\n"
             "  \"float bright\" [%2]\n"
             "  \"float contrast\" [%3]\n"
             "  \"string noisetype\" [\"%4\"]\n"
             "  \"string noisebasis\" [\"%5\"]\n"
             "  \"float noisesize\" [%6]\n"
             "  \"integer noisedepth\" [%7]"
             "  \"string coordinates\" [\"%8\"]\n"
             "  \"vector translate\" [0.0 0.0 0.0]\n"
             "  \"vector rotate\" [0.0 0.0 0.0]\n"
             "  \"vector scale\" [%9]"
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  
           .arg(tex->getBrightness())                                      
           .arg(tex->getContrast())                                        
           .arg(tex->usesHardNoise() ? "hard_noise" : "soft_noise")
           .arg(tex->getNoiseBasisAsString())                              
           .arg(tex->getNoiseSize())                                        
           .arg(tex->getNoiseDepth())                                      
           .arg(mapType)
           .arg(mapScale);

  };

};
}
#endif
