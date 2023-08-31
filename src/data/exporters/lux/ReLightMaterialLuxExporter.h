/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LIGHTMATERIAL_LUX_EXPORTER_H
#define RE_LIGHTMATERIAL_LUX_EXPORTER_H

#include <QString>

#include "reality_lib_export.h"
#include "exporters/lux/ReLuxMaterialExporter.h"

namespace boost {
  class any;
}

namespace Reality {
  class ReMaterial;
}


/**
 Exporter for the Light material
 */

namespace Reality {

class REALITY_LIB_EXPORT ReLightMaterialLuxExporter : public ReLuxMaterialExporter {

public:
  // Nothing to do because the mesh lights are actually exported in ReLuxGeometryExporter
 void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    result = QString();
  }
};

}

#endif
