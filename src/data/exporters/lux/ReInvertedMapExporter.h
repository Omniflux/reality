/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_INVERTEDMAP_TEXTURE_EXPORTER_H
#define LUX_INVERTEDMAP_TEXTURE_EXPORTER_H

#include "ReTexture.h"
#include "ReLuxTextureExporter.h"

namespace Reality {

/*
  Class: ReLuxInvertedMapExporter
 */

class RE_LIB_ACCESS ReLuxInvertedMapExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr tex, const QString& assignedName = "", bool isForPreview = false ) {
    return "Inverted texture export non implemented";
  };
};

}
#endif
