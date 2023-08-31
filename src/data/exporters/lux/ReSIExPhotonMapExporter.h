/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

// S.I. => Surface Integrator. For once the name is way too long to use
//         it expanded.

#ifndef RE_LUX_SIEXPHOTON_MAP_EXPORTER_H
#define RE_LUX_SIEXPHOTON_MAP_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReSIExporter.h"


namespace Reality {

class REALITY_LIB_EXPORT ReLuxSIExPhotonMapExporter : public ReLuxSIExporter {
  
public:

  // Constructor: ReLuxLightExporter
  ReLuxSIExPhotonMapExporter( const ReSurfaceIntegratorPtr si ) :
    ReLuxSIExporter(si)
  {

  };
  // Destructor: ReLuxLightExporter
  ~ReLuxSIExPhotonMapExporter() {
  }

  /**
   * This method returns the text, in Lux format, that exports the
   * Surface Integrator
   */
  QString exportSI() {
   QVariantMap vals = si->getValues();

  return( QString(
             "SurfaceIntegrator \"exphotonmap\" "
             "\"string renderingmode\" [\"%1\"] "
             "\"integer directphotons\" [%2] "
             "\"integer indirectphotons\" [%3] "
             "\"integer causticphotons\" [%4] "
             "\"integer maxdepth\" [%5]"
             "\"integer maxphotondepth\" [%6]\n"
          )
          .arg(vals[PHOTON_MAP_DIRECT_LIGHTING].toBool() ? "directlighting" : "path")
          .arg(vals[PHOTON_MAP_DIRECT_PHOTONS].toInt())
          .arg(vals[PHOTON_MAP_INDIRECT_PHOTONS].toInt())
          .arg(vals[PHOTON_MAP_CAUSTIC_PHOTONS].toInt())
          .arg(vals[PHOTON_MAP_MAX_DEPTH].toInt())
          .arg(vals[PHOTON_MAP_MAX_PHOTON_DEPTH].toInt())
        );
  };
};

} // namespace

#endif
