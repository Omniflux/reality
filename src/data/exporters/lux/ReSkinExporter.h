/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_SKIN_LUX_EXPORTER_H
#define RE_SKIN_LUX_EXPORTER_H

#include <boost/any.hpp>

#include "reality_lib_export.h"
#include "ReMaterial.h"
#include "ReSkin.h"
#include "exporters/lux/ReGlossyExporter.h"


namespace Reality {

/**
 * Lux exporter for the Skin material
 */
class REALITY_LIB_EXPORT ReSkinLuxExporter : public ReBasicGlossyLuxExporter<ReSkin> {

private:
  const QString addMaterial1( const QString& matName, 
                              const QString& bmTexName );
  const QString addMaterial2( const QString& matName, 
                              const QString& bmTexName );

protected:
  const QString exportWithSSS( const QString matName );
  const QString exportBaseMaterial( const QString matName );

public:
  static void makeHairMaskTexture( const QString& texName, 
                                   const QString& matName,
                                   const ReSkin* mat,
                                   QString& result );  
  void exportMaterial( const ReMaterial* basemat, boost::any& result );
};

}

#endif
