/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

// S.I. => Surface Integrator. For once the name is way too long to use
//         it expanded.

#ifndef RE_LUX_SI_SPPM_EXPORTER_H
#define RE_LUX_SI_SPPM_EXPORTER_H

#include "ReSIExporter.h"
#include "reality_lib_export.h"

namespace Reality {

class REALITY_LIB_EXPORT ReLuxSISPPMExporter : public ReLuxSIExporter {
  
public:

  // Constructor: ReLuxLightExporter
  ReLuxSISPPMExporter( const ReSurfaceIntegratorPtr si ) :
    ReLuxSIExporter(si)
  {

  };
  // Destructor: ReLuxLightExporter
  ~ReLuxSISPPMExporter() {
  }

  /**
   * This metod returns the text, in Lux format, that exports the
   * Surface Integrator
   */
  QString exportSI() {
    QVariantMap vals = si->getValues();

    return(QString(
             "SurfaceIntegrator \"sppm\" \n" 
             "  \"integer maxeyedepth\" [%1]\n" 
             "  \"integer maxphotondepth\" [%2]\n"
             "  \"integer photonperpass\" [%3] \n"
             "  \"float startradius\" [%4]\n"
             "  \"bool directlightsampling\" [\"%5\"]\n"
             "  \"float alpha\" [%6]\n"
             "  \"bool includeenvironment\" [\"%7\"]\n"
             "  \"string lookupaccel\" [\"%8\"]\n"
             "  \"string pixelsampler\" [\"%9\"]\n"
             "  \"string photonsampler\" [\"%10\"]\n"
           )
           .arg(vals[SPPM_MAX_EYE_DEPTH].toInt())
           .arg(vals[SPPM_MAX_PHOTON_DEPTH].toInt())
           .arg(vals[SPPM_PHOTONS_PER_PASS].toInt())
           .arg(vals[SPPM_START_RADIUS].toFloat())
           .arg(vals[SPPM_DIRECT_SAMPLING].toBool() ? "true" : "false")
           .arg(vals[SPPM_ALPHA].toFloat())
           .arg(vals[SPPM_INCLUDE_ENVIRONMENT].toBool() ? "true" : "false")
           .arg(vals[SPPM_LOOKUP_ACCELERATOR].toString())
           .arg(vals[SPPM_PIXEL_SAMPLER].toString())
           .arg(vals[SPPM_PHOTON_SAMPLER].toString()));
  };
};

} // namespace

#endif
