/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LIGHTMATERIAL_LUX_EXPORTER_H
#define RE_LIGHTMATERIAL_LUX_EXPORTER_H

#include "ReLightMaterial.h"
#include "exporters/lux/ReLuxMaterialExporter.h"

/**
 Exporter for the Light material
 */

namespace Reality {

class RE_LIB_ACCESS ReLightMaterialLuxExporter : public ReLuxMaterialExporter {

public:
  // Nothing to do becauxe the mesh lights are actually exported in ReLuxGeometryExporter
 void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    result = QString();
  }
};

}

#endif
