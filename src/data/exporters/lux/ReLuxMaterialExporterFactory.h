/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LUX_MAT_EXPORTER_FACTORY_H
#define RE_LUX_MAT_EXPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>

#include "ReMaterials.h"
#include "ReLuxMaterialExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 Factory that creates on-demand exporters for Lux Material exporters. Only exporters that are
 actually requested during a run of Reality are created. This is a singleton factory.
 */

typedef QSharedPointer<ReLuxMaterialExporter> ReLuxMaterialExporterPtr;

class REALITY_LIB_EXPORT ReLuxMaterialExporterFactory {
private:
  static QHash<int, ReLuxMaterialExporterPtr> exporters;

  static ReLuxMaterialExporterPtr getExporterService( const int matType );

public:
  static ReLuxMaterialExporterPtr getExporter( const ReMaterial* mat );

  //! Clears the cache of material converters. Usually called when we 
  //! change the value of the "Render as statue" flag.
  static void clearExporterCache();
};

}
#endif
