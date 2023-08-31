/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_GLOSSY_LUXCORE_EXPORTER_H
#define RE_GLOSSY_LUXCORE_EXPORTER_H

#include <boost/any.hpp>
#include <QString>

#include "reality_lib_export.h"
#include "exporters/luxcore/ReLuxcoreMaterialExporter.h"

namespace Reality {
  class ReGlossy;
  class ReMaterial;
}


/**
  Exports the Glossy material to Luxcore/SLG
 */
namespace Reality {

class REALITY_LIB_EXPORT ReGlossyLuxcoreExporter : public ReLuxcoreMaterialExporter
{
protected:
  const ReGlossy* mat;
  const QString exportBaseMaterial( const QString matName );
  void exportKs( QString& ksTextures, QString& ksValue );

public:
  void exportMaterial( const ReMaterial* basemat, boost::any& result );
};

}

#endif
