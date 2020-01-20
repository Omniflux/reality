#ifndef RE_QT_NULL_IMPORTER_H
#define RE_QT_NULL_IMPORTER_H

#include "ReNull.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>

/*
  Class: ReNullImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class ReQtNullImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    importBasicInformation(baseMat, data);
  }
  void importFromClipboard( ReMaterialPtr mat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {

    // nothing to do    
  };

};

}

#endif
