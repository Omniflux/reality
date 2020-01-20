/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_FBM_TEXTURE_EXPORTER_H
#define LUX_FBM_TEXTURE_EXPORTER_H

#include "textures/ReFBM.h"
#include "ReLuxTextureExporter.h"

namespace Reality {

/*
  Class: ReLuxFBMExporter
 */

class RE_LIB_ACCESS ReLuxFBMExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texBase, 
                               const QString& assignedName = "", 
                               bool isForPreview = false ) {
    ReFBMPtr tex = texBase.staticCast<ReFBM>();
    QString mapType = getMappingAsString(tex);
    QString mapScale = QString("%1 %1 %1").arg((double)tex->getScale(),0,'f',4);
    if (isForPreview) {
      mapType = "global";
    }
    QString str;
    QString chStr = findChannel(texBase);
    if (!chStr.isEmpty()) {
      str = QString("# From channel %1\n").arg(chStr);
    }

    if ( tex->getNamedValue("wrinkled").toBool() ) {
      str += QString(                   
               "Texture \"%1\" \"float\" \"wrinkled\"\n"
               "  \"integer octaves\" [%2]\n"
               "  \"float roughness\" [%3]\n"
               "  \"string coordinates\" [\"%4\"]\n"
               "  \"vector translate\" [0.0 0.0 0.0]\n"
               "  \"vector rotate\" [0.0 0.0 0.0]\n"
               "  \"vector scale\" [%5]"
             )
             .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
             .arg(tex->getNamedValue("octaves").toInt())
             .arg(tex->getNamedValue("roughness").toFloat())
             .arg(mapType)
             .arg(mapScale)
             ;        
    }
    // else
    str += QString(                   
             "Texture \"%1\" \"float\" \"fbm\"\n"
             "  \"integer octaves\" [%2]\n"
             "  \"float roughness\" [%3]\n"
             "  \"string coordinates\" [\"%4\"]\n"
             "  \"vector translate\" [0.0 0.0 0.0]\n"
             "  \"vector rotate\" [0.0 0.0 0.0]\n"
             "  \"vector scale\" [%5]"
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
           .arg(tex->getNamedValue("octaves").toInt())
           .arg(tex->getNamedValue("roughness").toFloat())
           .arg(mapType)
           .arg(mapScale)
           ;
    return str;
  };
};

}
#endif
