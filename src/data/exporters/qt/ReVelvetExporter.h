/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_VELVET_EXPORTER_H
#define RE_QT_VELVET_EXPORTER_H

#include "ReVelvet.h"
#include "ReQtMaterialExporter.h"

namespace Reality {

class RE_LIB_ACCESS ReQtVelvetExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatVelvet) {
      return;
    }
    const ReVelvet* mat = static_cast<const ReVelvet*>(basemat);
    addBasicInformation(mat);
    matData["thickness"] = mat->getThickness();

    exportAlphaChannel(mat);
    exportModifiers( mat);
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
