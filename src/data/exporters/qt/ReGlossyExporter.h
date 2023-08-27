/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_GLOSSY_EXPORTER_H
#define RE_QT_GLOSSY_EXPORTER_H

#include "ReGlossy.h"
#include "ReQtMaterialExporter.h"
#include "reality_lib_export.h"

namespace Reality {

class REALITY_LIB_EXPORT ReQtGlossyExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();

    if (basemat->getType() != MatGlossy) {
      return;
    }
    auto mat = static_cast<const Glossy*>(basemat);
    addBasicInformation(mat);
    matData["surfaceFuzz"]   = mat->getSurfaceFuzz();
    matData["uGlossiness"]   = mat->getUGlossiness();
    matData["vGlossiness"]   = mat->getVGlossiness();
    matData["coatThickness"] = mat->getCoatThickness();
    matData["translucent"]   = mat->isTranslucent();
    matData["topCoat"]       = mat->hasTopCoat();

    exportAlphaChannel(mat);
    exportModifiers(mat);
    boost::any textureData;
    exportTextures(mat, textureData);
    // The value returned by exportTextures() is a map containing
    // the definition of the textures and the channels. We need
    // to inject those two sets into the matData map
    QVariantMap tmp = boost::any_cast<QVariantMap>(textureData);
    matData["textures"] = tmp.value("textures");
    matData["channels"] = tmp.value("channels");
    result = matData;
  }
};

}

#endif
