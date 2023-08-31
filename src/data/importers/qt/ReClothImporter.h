#ifndef RE_QT_CLOTH_IMPORTER_H
#define RE_QT_CLOTH_IMPORTER_H

#include <QVariantMap>

#include "ReCloth.h"
#include "ReQtMaterialImporter.h"


namespace Reality {

/**
  Class: ReClothImporter

  Imports the material definition from a QVariantMap
 */
class ReQtClothImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReClothPtr mat = baseMat.staticCast<ReCloth>();
    importBasicInformation(mat, data);
    mat->setPresetName( data.value("presetName").toString() );
    mat->setURepeat( data.value("uRepeat").toFloat() );
    mat->setVRepeat( data.value("vRepeat").toFloat() );
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {

    ReClothPtr mat = baseMat.staticCast<ReCloth>();
    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setPresetName( data.value("presetName").toString() );
    mat->setURepeat( data.value("uRepeat").toFloat() );
    mat->setVRepeat( data.value("vRepeat").toFloat() );
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
