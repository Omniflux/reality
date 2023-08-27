/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LUX_TEX_EXPORTER_FACTORY_H
#define RE_LUX_TEX_EXPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>

#include "ReTexture.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/*
 Class: ReLuxTextureExporterFactory

 Factory that creates on-demand exporters for Lux texture exporters. Only exporters that are
 actually requested during a run of Reality are actually created. This is a singleton factory.
 */

class REALITY_LIB_EXPORT ReLuxTextureExporterFactory {
private:
  static QHash<int, ReLuxTextureExporterPtr> exporters;

  static ReLuxTextureExporterPtr getExporterService( const int expType);

public:
  static ReLuxTextureExporterPtr getExporter( const ReTexturePtr tex );

};

}
#endif
