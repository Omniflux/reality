/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef RE_QT_TEX_EXPORTER_FACTORY_H
#define RE_QT_TEX_EXPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>

#include "ReTexture.h"
#include "exporters/qt/ReQtTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * Factory that creates on-demand exporters for texture exporters. Only exporters
 * that are actually requested during a run of Reality are actually created. 
 * This is a singleton factory.
 */

class REALITY_LIB_EXPORT ReQtTextureExporterFactory {
private:
  static QHash<int, ReQtTextureExporterPtr> exporters;

  static ReQtTextureExporterPtr getExporterService( const int expType);

public:
  static ReQtTextureExporterPtr getExporter( const ReTexturePtr tex );

};

}
#endif
