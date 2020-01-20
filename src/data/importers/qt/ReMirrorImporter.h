#ifndef RE_QT_MIRROR_IMPORTER_H
#define RE_QT_MIRROR_IMPORTER_H

#include "ReMirror.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>

/*
  Class: ReMirrorImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class ReQtMirrorImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReMirrorPtr mat = baseMat.staticCast<ReMirror>();
    importBasicInformation(mat, data);
    mat->setFilmIOR(data.value("filmIOR").toFloat());
    mat->setFilmThickness(data.value("filmThickness").toFloat());
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    
    ReMirrorPtr mat = baseMat.staticCast<ReMirror>();

    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setFilmIOR(data.value("filmIOR").toFloat());
    mat->setFilmThickness(data.value("filmThickness").toFloat());
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
