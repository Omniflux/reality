/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_CHECKERS_TEXTURE_EXPORTER_H
#define LUX_CHECKERS_TEXTURE_EXPORTER_H

#include "textures/ReCheckers.h"
#include "ReLuxTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
  Class: ReLuxCheckersExporter
 */
class REALITY_LIB_EXPORT ReLuxCheckersExporter : public ReLuxTextureExporter{

public:
  const QString exportTexture( ReTexturePtr texBase, 
                               const QString& assignedName = "", 
                               bool isForPreview = false ) 
  {
    ReCheckersPtr tex = texBase.staticCast<ReCheckers>();
    QString mapType = getMappingAsString(tex);
    QString mapScale = QString("%1 %1 %1").arg((double)tex->getScale(),0,'f',4);
    bool is3D = tex->is3D();
    if ( isForPreview && is3D ) {
      mapType = "global";
    }

    auto tex1 = ReLuxTextureExporter::getTextureFromCache(tex->getTex1());
    auto tex2 = ReLuxTextureExporter::getTextureFromCache(tex->getTex2());
    QString str = QString(                   
             "Texture \"%1\" \"%2\" \"checkerboard\"\n"
             "  \"integer dimension\" [%3]\n"
             "  \"texture tex1\" [\"%4\"]\n"
             "  \"texture tex2\" [\"%5\"]\n"
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
           .arg(tex->getDataTypeAsString())
           .arg(is3D ? "3" : "2")
           .arg(tex1->getUniqueName())
           .arg(tex2->getUniqueName())
           ;  
    if ( is3D ) {
      str += QString(
               "  \"string coordinates\" [\"%1\"]\n"
               "  \"vector translate\" [0.0 0.0 0.0]\n"
               "  \"vector rotate\" [0.0 0.0 0.0]\n"
               "  \"vector scale\" [%2]"
             )
             .arg(mapType)
             .arg(mapScale);
    }
    else {
      str += QString("  \"string mapping\" [\"uv\"]");
    }
    return str;
  };  
};

}
#endif
