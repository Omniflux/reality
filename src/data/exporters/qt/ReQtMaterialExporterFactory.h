/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MAT_EXPORTER_FACTORY_H
#define RE_QT_MAT_EXPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>

#include "reality_lib_export.h"

namespace Reality {
  class ReMaterial;
  class ReQtMaterialExporter;
  typedef QSharedPointer<ReQtMaterialExporter> ReQtMaterialExporterPtr;
}

namespace Reality {

/**
 * Factory that creates on-demand exporters for Qt Material exporters. Only exporters that are
 * actually requested during a run of Reality are created. This is a singleton factory.
 */

class REALITY_LIB_EXPORT ReQtMaterialExporterFactory {
private:
  static QHash<int, ReQtMaterialExporterPtr> exporters;

  static ReQtMaterialExporterPtr getExporterService( const int matType );

public:
  static ReQtMaterialExporterPtr getExporter( const ReMaterial* mat );
};

}
#endif
