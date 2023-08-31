/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MATTE_IMPORTER_H
#define RE_QT_MATTE_IMPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReMatte.h"
#include "ReQtMaterialImporter.h"


/**
  Class: ReMatteImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class REALITY_LIB_EXPORT ReQtMatteImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReMattePtr mat = baseMat.staticCast<ReMatte>();
    importBasicInformation(mat, data);
    mat->setRoughness( data.value("roughness").toFloat() );
    mat->setConserveEnergy( data.value("conserveEnergy").toBool() );
    mat->setTranslucent( data.value("translucent").toBool() );
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {

    ReMattePtr mat = baseMat.staticCast<ReMatte>();
    
    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setConserveEnergy( data.value("conserveEnergy").toBool() );
    mat->setTranslucent( data.value("translucent").toBool() );
    if ( texMode != Keep ) {
      importTextures(mat, data, texMode);
    }
    importAlphaChannel(mat, data);
    importModifiers(mat, data);

    mat->setEdited();
  };

};

}

#endif
