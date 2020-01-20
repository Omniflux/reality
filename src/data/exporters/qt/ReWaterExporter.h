/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_WATER_EXPORTER_H
#define RE_QT_WATER_EXPORTER_H

#include "ReWater.h"
#include "ReQtMaterialExporter.h"

namespace Reality {

class RE_LIB_ACCESS ReQtWaterExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatWater) {
      return;
    }
    auto mat = static_cast<const ReWater*>(basemat);
    addBasicInformation(mat);
    matData["ripples"]      = mat->getRipples();
    matData["ripplePreset"] = mat->getPresetNumber();
    matData["IOR"] = mat->getIOR();
    matData["Kt"] = mat->getKt().name();
    matData["clarityAtDepth"] = mat->getClarityAtDepth();

    exportAlphaChannel(mat);
    exportModifiers(mat);
    boost::any textureData;
    exportTextures(mat, textureData);
    QVariantMap texData = boost::any_cast<QVariantMap>(textureData);
    matData["textures"] = texData.value("textures");
    matData["channels"] = texData.value("channels");
    result = matData;
  }
};

}

#endif
