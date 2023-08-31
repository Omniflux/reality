#ifndef RE_QT_METAL_IMPORTER_H
#define RE_QT_METAL_IMPORTER_H

#include <QVariantMap>

#include "ReMetal.h"
#include "ReQtMaterialImporter.h"


namespace Reality {

/**
  Class: ReMetalImporter

  Imports the material definition from a QVariantMap
 */
class ReQtMetalImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReMetalPtr mat = baseMat.staticCast<ReMetal>();
    importBasicInformation(mat, data);
    mat->setMetalType( static_cast<ReMetal::MetalType>(data.value("metalType").toInt()) );
    mat->setHPolish(data.value("hPolish").toFloat());
    mat->setVPolish(data.value("vPolish").toFloat());
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    
    ReMetalPtr mat = baseMat.staticCast<ReMetal>();
    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setMetalType( static_cast<ReMetal::MetalType>(data.value("metalType").toInt()) );
    mat->setHPolish(data.value("hPolish").toFloat());
    mat->setVPolish(data.value("vPolish").toFloat());
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
