/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_GLOSSY_LUXCORE_EXPORTER_H
#define RE_GLOSSY_LUXCORE_EXPORTER_H

#include "ReGlossy.h"

// We simply re-use the Lux matte material. It is identical to what we need
#include "exporters/lux/ReGlossyExporter.h"
#include "exporters/luxcore/ReLuxcoreMaterialExporter.h"
#include "reality_lib_export.h"

/**
  Exports the Glossy material to Luxcore/SLG
 */
namespace Reality {

class REALITY_LIB_EXPORT ReGlossyLuxcoreExporter : public ReLuxcoreMaterialExporter
{
protected:
  const Glossy* mat;
  const QString exportBaseMaterial( const QString matName );
  void exportKs( QString& ksTextures, QString& ksValue );

public:
  void exportMaterial( const ReMaterial* basemat, boost::any& result );
};

}

#endif
