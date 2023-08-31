/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_WOOD_TEXTURE_EXPORTER_H
#define LUX_WOOD_TEXTURE_EXPORTER_H

#include "reality_lib_export.h"
#include "ReLuxTextureExporter.h"
#include "textures/ReWood.h"


namespace Reality {

/*
  Class: ReLuxWoodExporter
 */

class REALITY_LIB_EXPORT ReLuxWoodExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texBase, const QString& assignedName = "", bool isForPreview = false ) {
    ReWoodPtr tex = texBase.staticCast<ReWood>();    

    QString mapType = getMappingAsString(tex);
    QString mapScale = QString("%1 %1 %1").arg((double)tex->getScale(),0,'f',4);
    if (isForPreview) {
      mapType = "global";
      // We double the scale to make it more visible in preview
      mapScale = QString("%1 %1 %1").arg((double)tex->getScale() * 2,0,'f',4);
    }
    return QString(                   
             "Texture \"%1\" \"float\" \"blender_wood\"\n"
             "  \"float bright\" [%2]\n"
             "  \"float contrast\" [%3]\n"
             "  \"string type\" [\"%4\"]\n"
             "  \"string noisetype\" [\"%5\"]\n"
             "  \"string noisebasis\" [\"%6\"]\n"
             "  \"string noisebasis2\" [\"%7\"]\n"
             "  \"float noisesize\" [%8]\n"
             "  \"float turbulence\" [%9]\n"
             "  \"string coordinates\" [\"%10\"]\n"
             "  \"vector translate\" [0.0 0.0 0.0]\n"
             "  \"vector rotate\" [0.0 0.0 0.0]\n"
             "  \"vector scale\" [%11]"
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  
           .arg(tex->getBrightness())                                      
           .arg(tex->getContrast()) 
           .arg(tex->getWoodPatternAsString())
           .arg(tex->hasSoftNoise() ? "soft_noise" : "hard_noise" )
           .arg(tex->getNoiseBasisAsString())
           .arg(tex->getVeinWaveAsString())
           .arg(tex->getNoiseSize())                                        
           .arg(tex->getTurbulence())
           .arg(mapType)
           .arg(mapScale);
  };
};

}
#endif
