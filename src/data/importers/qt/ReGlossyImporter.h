#ifndef RE_QT_GLOSSY_IMPORTER_H
#define RE_QT_GLOSSY_IMPORTER_H

#include "ReGlossy.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>

/*
  Class: ReGlossyImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class RE_LIB_ACCESS ReQtGlossyImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReGlossyPtr mat = baseMat.staticCast<Glossy>();
    importBasicInformation(mat, data);
    mat->setSurfaceFuzz( data.value("surfaceFuzz").toBool() );
    mat->setUGlossiness(data.value("uGlossiness").toFloat());
    mat->setVGlossiness(data.value("vGlossiness").toFloat());
    mat->setCoatThickness(data.value("coatThickness").toFloat());
    mat->setTranslucent(data.value("translucent").toBool());
    mat->setTopCoat(data.value("topCoat").toBool());
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }


  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    ReGlossyPtr mat = baseMat.staticCast<Glossy>();

    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setSurfaceFuzz(data.value("surfaceFuzz").toBool());
    mat->setUGlossiness(data.value("uGlossiness").toFloat());
    mat->setVGlossiness(data.value("vGlossiness").toFloat());
    mat->setCoatThickness(data.value("coatThickness").toFloat());
    mat->setTranslucent(data.value("translucent").toBool());
    mat->setTopCoat(data.value("topCoat").toBool());
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
