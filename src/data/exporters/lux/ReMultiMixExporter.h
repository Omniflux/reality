/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_MULTIMIX_TEXTURE_EXPORTER_H
#define LUX_MULTIMIX_TEXTURE_EXPORTER_H

#include "ReTexture.h"
#include "ReLuxTextureExporter.h"

namespace Reality {

/*
  Class: ReLuxMultiMixExporter
 */

class RE_LIB_ACCESS ReLuxMultiMixExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr tex, const QString& assignedName = "", bool isForPreview = false ) {
    return "Multi Mix texture export non implemented";
  };
};

}
#endif
