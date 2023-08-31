/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MIX_LUX_EXPORTER_H
#define RE_MIX_LUX_EXPORTER_H

#include "reality_lib_export.h"
#include "exporters/lux/ReLuxMaterialExporter.h"

namespace boost {
  class any;
}

namespace Reality {
  class ReMaterial;
}


/*
  Class: ReMixLuxExporter
 */

namespace Reality {

class REALITY_LIB_EXPORT ReMixLuxExporter : public ReLuxMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result );
};

}

#endif
