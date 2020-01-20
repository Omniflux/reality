/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MATTE_EXPORTER_H
#define RE_QT_MATTE_EXPORTER_H

#include "ReMatte.h"
#include "ReQtMaterialExporter.h"

namespace Reality {

class RE_LIB_ACCESS ReQtMatteExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatMatte) {
      return;
    }
    auto mat = static_cast<const ReMatte*>(basemat);
    addBasicInformation(mat);
    matData["roughness"]      = mat->getRoughness();
    matData["conserveEnergy"] = mat->conservesEnergy();
    matData["translucent"]    = mat->isTranslucent();

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
