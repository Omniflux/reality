/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

// S.I. => Surface Integrator. For once the name is way too long to use
//         it expanded.

#ifndef RE_LUX_SIBIDIR_EXPORTER_H
#define RE_LUX_SIBIDIR_EXPORTER_H

#include "ReSIExporter.h"

namespace Reality {

/**
  Base calss for all Surface Integrator exporters.
 */
class RE_LIB_ACCESS ReLuxSIBidirExporter : public ReLuxSIExporter {
  
public:

  // Constructor: ReLuxLightExporter
  ReLuxSIBidirExporter( const ReSurfaceIntegratorPtr si ) :
    ReLuxSIExporter(si)
  {

  };
  // Destructor: ReLuxLightExporter
  ~ReLuxSIBidirExporter() {
  }

  /**
   * This metod returns the text, in Lux format, that exports the
   * Surface Integrator
   */
   QString exportSI() {
     QVariantMap vals = si->getValues();
     uint eyeRR = vals[BIDIR_EYE_RR].toInt();
     uint lightRR = vals[BIDIR_LIGHT_RR].toInt();
     if (eyeRR && lightRR) {
       return(QString(
                "SurfaceIntegrator \"bidirectional\" \"integer eyedepth\" [%1] "
                "\"integer lightdepth\" [%2] "
                "\"string strategy\" [\"%3\"] "
                "\"integer eyerrthreshold\" [%4] "
                "\"integer lightrrthreshold\" [%5]\n"
              )
              .arg(vals[BIDIR_EYE_DEPTH].toInt())
              .arg(vals[BIDIR_LIGHT_DEPTH].toInt())
              .arg(vals[BIDIR_LIGHT_STRATEGY].toString())
              .arg(eyeRR)
              .arg(lightRR)                  
              );
     }
     else {
       return(QString(
                "SurfaceIntegrator \"bidirectional\" \"integer eyedepth\" [%1] "
                "\"integer lightdepth\" [%2] "
                "\"string strategy\" [\"%3\"]\n"
              )
              .arg(vals[BIDIR_EYE_DEPTH].toInt())
              .arg(vals[BIDIR_LIGHT_DEPTH].toInt())
              .arg(vals[BIDIR_LIGHT_STRATEGY].toString())
              );      
     }

   };
};

} // namespace

#endif
