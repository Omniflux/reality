/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

// S.I. => Surface Integrator. For once the name is way too long to use
//         it expanded.

#ifndef RE_LUX_SI_DIRECT_EXPORTER_H
#define RE_LUX_SI_DIRECT_EXPORTER_H

#include "ReSIExporter.h"
#include "reality_lib_export.h"

namespace Reality {

class REALITY_LIB_EXPORT ReLuxSIDirectExporter : public ReLuxSIExporter {
  
public:

  // Constructor: ReLuxLightExporter
  ReLuxSIDirectExporter( const ReSurfaceIntegratorPtr si ) :
    ReLuxSIExporter(si)
  {

  };
  // Destructor: ReLuxLightExporter
  ~ReLuxSIDirectExporter() {
  }

  /**
   * This metod returns the text, in Lux format, that exports the
   * Surface Integrator
   */
  QString exportSI() {
    QVariantMap vals = si->getValues();

    return(QString("SurfaceIntegrator \"directlighting\" \"integer maxdepth\" [%1]\n")
           .arg(vals[DIRECT_MAX_DEPTH].toInt()));
  };
};

} // namespace

#endif
