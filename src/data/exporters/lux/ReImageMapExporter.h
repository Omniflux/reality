/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_IMAGEMAP_TEXTURE_EXPORTER_H
#define LUX_IMAGEMAP_TEXTURE_EXPORTER_H

#include <QString>

#include "reality_lib_export.h"
#include "ReLuxTextureExporter.h"

namespace Reality {
  class ReTexture;
  typedef QSharedPointer<ReTexture> ReTexturePtr;
}


namespace Reality {

/*
  Class: ReLuxImageMapExporter
 */

class REALITY_LIB_EXPORT ReLuxImageMapExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texbase, const QString& assignedName = "", bool isForPreview = false );
};

}
#endif
