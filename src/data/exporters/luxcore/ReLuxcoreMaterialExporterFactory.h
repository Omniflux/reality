/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LUXCORE_MAT_EXPORTER_FACTORY_H
#define RE_LUXCORE_MAT_EXPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>

#include "ReMaterials.h"
#include "ReLuxcoreMaterialExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 Factory that creates on-demand exporters for Lux Material exporters. Only exporters that are
 actually requested during a run of Reality are created. This is a singleton factory.
 */

typedef QSharedPointer<ReLuxcoreMaterialExporter> ReLuxcoreMaterialExporterPtr;
typedef QHash<int, ReMaterialExporterPtr> ExportersMap;

class REALITY_LIB_EXPORT ReLuxcoreMaterialExporterFactory {
private:
  static ExportersMap exporters;

  static ReMaterialExporterPtr getExporterService( const int matType );

public:
  static ReMaterialExporterPtr getExporter( const ReMaterial* mat );

  //! Clears the cache of material converters. Usually called when we 
  //! change the value of the "Render as statue" flag.
  static void clearExporterCache();
};

}
#endif
