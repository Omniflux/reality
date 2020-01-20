/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MAT_IMPORTER_FACTORY_H
#define RE_QT_MAT_IMPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>

#include "ReMaterials.h"
#include "ReQtMaterialImporter.h"

namespace Reality {

/*
 Class: ReQtMaterialImporterFactory

 Factory that creates on-demand importers based on QVariantMap. Only importers that are
 actually requested during a run of Reality are created. This is a singleton factory.
 */

typedef QSharedPointer<ReQtMaterialImporter> ReQtMaterialImporterPtr;

class RE_LIB_ACCESS ReQtMaterialImporterFactory {
private:
  static QHash<int, ReQtMaterialImporterPtr> importers;

  static ReQtMaterialImporterPtr getImporterService( const ReMaterialType matType );

public:
  static ReQtMaterialImporterPtr getImporter( const ReMaterialType matType );

};

}
#endif
