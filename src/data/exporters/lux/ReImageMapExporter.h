/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_IMAGEMAP_TEXTURE_EXPORTER_H
#define LUX_IMAGEMAP_TEXTURE_EXPORTER_H

#include "ReTexture.h"
#include "ReLuxTextureExporter.h"

namespace Reality {

/*
  Class: ReLuxImageMapExporter
 */

class RE_LIB_ACCESS ReLuxImageMapExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texbase, const QString& assignedName = "", bool isForPreview = false );
};

}
#endif
