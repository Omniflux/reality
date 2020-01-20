/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MAT_EXPORTER_FACTORY_H
#define RE_QT_MAT_EXPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>

#include "ReMaterials.h"
#include "ReQtMaterialExporter.h"

namespace Reality {

/**
 * Factory that creates on-demand exporters for Qt Material exporters. Only exporters that are
 * actually requested during a run of Reality are created. This is a singleton factory.
 */

typedef QSharedPointer<ReQtMaterialExporter> ReQtMaterialExporterPtr;

class RE_LIB_ACCESS ReQtMaterialExporterFactory {
private:
  static QHash<int, ReQtMaterialExporterPtr> exporters;

  static ReQtMaterialExporterPtr getExporterService( const int matType );

public:
  static ReQtMaterialExporterPtr getExporter( const ReMaterial* mat );
};

}
#endif
