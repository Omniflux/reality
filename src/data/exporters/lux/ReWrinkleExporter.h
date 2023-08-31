/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_WRINKLE_TEXTURE_EXPORTER_H
#define LUX_WRINKLE_TEXTURE_EXPORTER_H

#include <QSharedPointer>

#include "reality_lib_export.h"
#include "ReLuxTextureExporter.h"

namespace Reality {
  class ReTexture;
  typedef QSharedPointer<ReTexture> ReTexturePtr;
}


namespace Reality {

/*
  Class: ReLuxWrinkleExporter
 */

class REALITY_LIB_EXPORT ReLuxWrinkleExporter : public ReLuxTextureExporter {

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
