/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MATTE_LUXCORE_EXPORTER_H
#define RE_MATTE_LUXCORE_EXPORTER_H

// We simply re-use the Lux matte material. It is identical to what we need
#include "exporters/lux/ReMatteExporter.h"
#include "exporters/luxcore/ReLuxcoreMaterialExporter.h"

/**
  Exports the Matte material to Lux
 */
namespace Reality {

class RE_LIB_ACCESS ReMatteLuxcoreExporter : public ReMatteLuxExporter 
{

};

}

#endif
