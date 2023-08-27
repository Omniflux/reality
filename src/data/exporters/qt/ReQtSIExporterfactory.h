/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_SI_EXPORTER_FACTORY_H
#define RE_QT_SI_EXPORTER_FACTORY_H

#include "ReQtSIExporter.h"
#include "reality_lib_export.h"

namespace Reality {

class REALITY_LIB_EXPORT ReQtSIExporterFactory {
public:
  // Constructor: ReQtSIExporterFactory
  ReQtSIExporterFactory() {
  };
  // Destructor: ReQtSIExporterFactory
 ~ReQtSIExporterFactory() {
  };

  static ReQtSIExporterPtr getExporter( const ReSurfaceIntegratorPtr si ) {
    // We store the S.I. internally as a QVariantMap so there is no 
    // transformation needed. One exporter works for all different types.
    // No leaking because we are using a smart pointer
    return ReQtSIExporterPtr( new ReQtSIExporter(si) );
  }
  
};


} // namespace

#endif
