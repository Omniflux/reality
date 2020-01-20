#ifndef RE_QT_VELVET_IMPORTER_H
#define RE_QT_VELVET_IMPORTER_H

#include "ReVelvet.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>

/*
  Class: ReVelvetImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class ReQtVelvetImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReVelvetPtr mat = baseMat.staticCast<ReVelvet>();
    importBasicInformation(mat, data);
    mat->setThickness( data.value("thickness").toFloat() );
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    
    ReVelvetPtr mat = baseMat.staticCast<ReVelvet>();

    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setThickness( data.value("thickness").toFloat() );
    importModifiers( mat, data );
    importAlphaChannel( mat, data );
    if ( texMode != Keep ) {
      importTextures(mat, data, texMode);
    }

    mat->setEdited();
  };

};

}

#endif
