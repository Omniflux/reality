/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_MARBLE_TEXTURE_EXPORTER_H
#define LUX_MARBLE_TEXTURE_EXPORTER_H

#include "textures/ReMarble.h"
#include "ReLuxTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/*
  Class: ReLuxMarbleExporter
 */

class REALITY_LIB_EXPORT ReLuxMarbleExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texBase, const QString& assignedName = "", bool isForPreview = false ) {
    ReMarblePtr tex = texBase.staticCast<ReMarble>();    

    QString mapType = getMappingAsString(tex);
    QString mapScale = QString("%1 %1 %1").arg((double)tex->getScale(),0,'f',4);
    if (isForPreview) {
      mapType = "global";
      // We double the scale to make it more visible in preview
      mapScale = QString("%1 %1 %1").arg((double)tex->getScale() * 2,0,'f',4);
    }
    return QString(                   
             "Texture \"%1\" \"float\" \"blender_marble\"\n"
             "  \"float bright\" [%2]\n"
             "  \"float contrast\" [%3]\n"
             "  \"string type\" [\"%4\"]\n"
             "  \"string noisetype\" [\"%5\"]\n"
             "  \"string noisebasis\" [\"%6\"]\n"
             "  \"string noisebasis2\" [\"%7\"]\n"
             "  \"float noisesize\" [%8]\n"
             "  \"integer noisedepth\" [%9]\n"
             "  \"float turbulence\" [%10]\n"
             "  \"string coordinates\" [\"%11\"]\n"
             "  \"vector translate\" [0.0 0.0 0.0]\n"
             "  \"vector rotate\" [0.0 0.0 0.0]\n"
             "  \"vector scale\" [%12]"
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  
           .arg(tex->getBrightness())                                      
           .arg(tex->getContrast()) 
           .arg(tex->getVeinQualityAsString())
           .arg(tex->hasSoftNoise() ? "soft_noise" : "hard_noise" )
           .arg(tex->getNoiseBasisAsString())
           .arg(tex->getVeinWaveAsString())
           .arg(tex->getNoiseSize())                                        
           .arg(tex->getDepth())
           .arg(tex->getTurbulence())
           .arg(mapType)
           .arg(mapScale);
  };
};

}
#endif
