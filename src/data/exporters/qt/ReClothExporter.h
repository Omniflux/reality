/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_CLOTH_EXPORTER_H
#define RE_QT_CLOTH_EXPORTER_H

#include "ReCloth.h"
#include "ReQtMaterialExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * This class exports the cloth material to the QVariantMap format
 */
class REALITY_LIB_EXPORT ReQtClothExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatCloth) {
      return;
    }
    const ReCloth* mat = static_cast<const ReCloth*>(basemat);
    QString matName = mat->getUniqueName();
    addBasicInformation(mat);
    matData["presetName"] = mat->getPresetName();
    matData["uRepeat"] = mat->getURepeat();
    matData["vRepeat"] = mat->getVRepeat();

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
