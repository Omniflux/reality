#ifndef RE_QT_MI_IMPORTER_H
#define RE_QT_MI_IMPORTER_H

#include <QVariantMap>

#include "ReMix.h"
#include "ReQtMaterialImporter.h"


/*
  Class: ReMirrorImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class ReQtMixImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReMixPtr mat = baseMat.staticCast<ReMix>();
    importBasicInformation(mat, data);
    
    // Finish!

  }

};

}

#endif
