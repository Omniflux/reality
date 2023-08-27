/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_SI_EXPORTER_FACTORY_H
#define RE_SI_EXPORTER_FACTORY_H

#include "ReSIBidirExporter.h"
#include "ReSIPathExporter.h"
#include "ReSIPathGPUExporter.h"
#include "ReSISPPMExporter.h"
#include "ReSIExPhotonMapExporter.h"
#include "ReSIDirectExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/*
  Class: ReLuxSIExporterFactory
 */
class REALITY_LIB_EXPORT ReLuxSIExporterFactory {

public:
  // Constructor: ReLuxSIExporterFactory
  ReLuxSIExporterFactory() {
  };
  // Destructor: ReLuxSIExporterFactory
 ~ReLuxSIExporterFactory() {
  };

  static ReLuxSIExporterPtr getExporter( const ReSurfaceIntegratorPtr si ) {
    switch(si->getType()) {
      case SI_BIDIR:
        return ReLuxSIExporterPtr( new ReLuxSIBidirExporter(si) );
      case SI_PATH:
        return ReLuxSIExporterPtr( new ReLuxSIPathExporter(si) );
      case SI_PATH_GPU:
        return ReLuxSIExporterPtr( new ReLuxSIPathGPUExporter(si) );
      case SI_SPPM:
        return ReLuxSIExporterPtr( new ReLuxSISPPMExporter(si) );
      case SI_EXPM:
        return ReLuxSIExporterPtr( new ReLuxSIExPhotonMapExporter(si) );
      case SI_DIRECT:
        return ReLuxSIExporterPtr( new ReLuxSIDirectExporter(si) );
    }
    return ReLuxSIExporterPtr( new ReLuxSIBidirExporter(si) );
  }
  
};


} // namespace

#endif
