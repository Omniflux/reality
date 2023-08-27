/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

// S.I. => Surface Integrator. For once the name is way too long to use
//         it expanded.

#ifndef RE_LUX_SIPATHGPU_EXPORTER_H
#define RE_LUX_SIPATHGPU_EXPORTER_H

#include "ReSIExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
  Base calss for all Surface Integrator exporters.
 */
class REALITY_LIB_EXPORT ReLuxSIPathGPUExporter : public ReLuxSIExporter {
  
public:

  // Constructor: ReLuxLightExporter
  ReLuxSIPathGPUExporter( const ReSurfaceIntegratorPtr si ) :
    ReLuxSIExporter(si)
  {

  };
  // Destructor: ReLuxLightExporter
  ~ReLuxSIPathGPUExporter() {
  }

  /**
   * This metod returns the text, in Lux format, that exports the
   * Surface Integrator
   */
   QString exportSI() {
     QVariantMap vals = si->getValues();

     return(QString(
              "SurfaceIntegrator \"path\" \"integer maxdepth\" [%1] "
              "\"string lightstrategy\" [\"%2\"] "
              "\"bool includeenvironment\" [\"true\"] "
              "\"string rrstrategy\" [\"%3\"] "
              "\"integer shadowraycount\" [1]\n"
              )
            .arg(vals[PATH_MAX_DEPTH].toInt())
            .arg("one")
            .arg(vals[PATH_LIGHT_STRATEGY].toString()));
   };
};

} // namespace

#endif
