/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_GLASS_IMPORTER_H
#define RE_QT_GLASS_IMPORTER_H

#include "ReGlass.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>

/*
  Class: ReGlassImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class RE_LIB_ACCESS ReQtGlassImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReGlassPtr mat = baseMat.staticCast<ReGlass>();
    importBasicInformation(mat, data);
    mat->setGlassType(static_cast<ReGlass::GlassType>(data.value("glassType").toInt()));
    mat->setURoughness(data.value("uRoughness").toFloat());
    mat->setVRoughness(data.value("vRoughness").toFloat());
    mat->setThinFilmIOR(data.value("thinFilmIOR").toFloat());
    mat->setThinFilmThickness(data.value("thinFilmThickness").toFloat());
    mat->setIOR(data.value("IOR").toFloat());
    mat->setIORLabel(data.value("IORLabel").toString());
    mat->setDispersion(data.value("dispersion", false).toBool());
    mat->setCauchyB(data.value("cauchyB", RE_GLASS_DEFAULT_CAUCHYB).toFloat());
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    ReGlassPtr mat = baseMat.staticCast<ReGlass>();

    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setGlassType(static_cast<ReGlass::GlassType>(data.value("glassType").toInt()));
    mat->setURoughness(data.value("uRoughness").toFloat());
    mat->setVRoughness(data.value("vRoughness").toFloat());
    mat->setThinFilmIOR(data.value("thinFilmIOR").toFloat());
    mat->setThinFilmThickness(data.value("thinFilmThickness").toFloat());
    mat->setIOR(data.value("IOR").toFloat());
    mat->setIORLabel(data.value("IORLabel").toString());
    mat->setDispersion(data.value("dispersion", false).toBool());
    mat->setCauchyB(data.value("cauchyB", RE_GLASS_DEFAULT_CAUCHYB).toFloat());    
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
