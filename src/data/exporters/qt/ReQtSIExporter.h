/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

// S.I. => Surface Integrator. For once the name is way too long for using
//         it in expanded form.

#ifndef RE_QT_SI_EXPORTER_H
#define RE_QT_SI_EXPORTER_H

#include <QSharedPointer>
#include <QVariantMap>

#include "reality_lib_export.h"

namespace Reality {
  class ReSurfaceIntegrator;
  typedef QSharedPointer<ReSurfaceIntegrator> ReSurfaceIntegratorPtr;
}


namespace Reality {

/**
  Base class for all Surface Integrator exporters.
 */
class REALITY_LIB_EXPORT ReQtSIExporter {
protected:
  ReSurfaceIntegratorPtr si;
  
public:

  // Constructor: ReQtLightExporter
  ReQtSIExporter( const ReSurfaceIntegratorPtr si ) : si(si)
  {
  };

  // Destructor: ReQtLightExporter
  virtual ~ReQtSIExporter() {
  }

  /**
   * This metod returns the map, in Qt format, that exports the
   * Surface Integrator
   */
  virtual QVariantMap exportSI() {
    QVariantMap vals = si->getValues();

    vals["type"] = si->getType(); // Copy on write
    return vals;    
  };
};

typedef QSharedPointer<ReQtSIExporter> ReQtSIExporterPtr;

} // namespace

#endif
