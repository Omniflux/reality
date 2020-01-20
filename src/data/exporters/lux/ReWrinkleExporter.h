/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_WRINKLE_TEXTURE_EXPORTER_H
#define LUX_WRINKLE_TEXTURE_EXPORTER_H

#include "ReTexture.h"
#include "ReLuxTextureExporter.h"

namespace Reality {

/*
  Class: ReLuxWrinkleExporter
 */

class RE_LIB_ACCESS ReLuxWrinkleExporter : public ReLuxTextureExporter {

public:

  const QString exportTexture( ReTexturePtr tex,
                               const QString& assignedName = "",
                               bool isForPreview = false )
  {
    return "Wrinkle texture export non implemented";
  };
};

}
#endif
