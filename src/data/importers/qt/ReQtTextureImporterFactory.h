/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_TEX_IMPORTER_FACTORY_H
#define RE_QT_TEX_IMPORTER_FACTORY_H

#include <QHash>

#include "reality_lib_export.h"
#include "ReDefs.h"

namespace Reality {
  class ReQtTextureImporter;
  typedef QSharedPointer<ReQtTextureImporter> ReQtTextureImporterPtr;
}


namespace Reality {

/**
 * Factory that creates on-demand importers for restoring textures based on QVariantMaps. 
 * Only importers that are actually requested during a run of Reality are actually created.
 * This is a singleton factory.
 */

class REALITY_LIB_EXPORT ReQtTextureImporterFactory {
private:
  // Importer table, based on the texture type
  static QHash<int, ReQtTextureImporterPtr> importers;

  static ReQtTextureImporterPtr getImporterService( const int expType );

public:
  static ReQtTextureImporterPtr getImporter( const ReTextureType texType );

};

}
#endif
