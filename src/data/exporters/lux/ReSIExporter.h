/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

// S.I. => Surface Integrator. For once the name is way too long to use
//         it expanded.

#ifndef RE_LUX_SI_EXPORTER_H
#define RE_LUX_SI_EXPORTER_H

#include "ReDefs.h"
#include "ReSurfaceIntegrator.h"
#include "reality_lib_export.h"

namespace Reality {

/**
  Base class for all Surface Integrator exporters.
 */
class REALITY_LIB_EXPORT ReLuxSIExporter {
protected:
  ReSurfaceIntegratorPtr si;
  
public:

  // Constructor: ReLuxLightExporter
  ReLuxSIExporter( const ReSurfaceIntegratorPtr si ) :
    si(si)
  {

  };
  // Destructor: ReLuxLightExporter
  virtual ~ReLuxSIExporter() {
  }

  /**
   * This metod returns the text, in Lux format, that exports the
   * Surface Integrator
   */
  virtual QString exportSI() = 0;
};

typedef QSharedPointer<ReLuxSIExporter> ReLuxSIExporterPtr;

} // namespace

#endif
