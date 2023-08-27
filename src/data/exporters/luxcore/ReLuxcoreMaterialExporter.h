/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LUXCORE_MATERIAL_EXPORTER_H
#define RE_LUXCORE_MATERIAL_EXPORTER_H

#include "exporters/lux/ReLuxMaterialExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/*
  Class: ReLuxCoreMateriaExporter

  Specializes the abstract <ReMaterialExporter> to handle Luxcore/SLG export functions.
  All specialized Luxcore material exporters are descendants of this class.

  It's a descendant of the ReLuxMaterialExported because that class has already methods
  for generating alpha channel, bump map and displacement map declarations. 
 */

class REALITY_LIB_EXPORT ReLuxcoreMaterialExporter : public ReLuxMaterialExporter {

};

}

#endif
