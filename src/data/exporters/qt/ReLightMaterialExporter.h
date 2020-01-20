/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_LIGHTMATERIAL_EXPORTER_H
#define RE_QT_LIGHTMATERIAL_EXPORTER_H

#include "ReLightMaterial.h"
#include "ReQtMaterialExporter.h"


namespace Reality {

class RE_LIB_ACCESS ReQtLightMaterialExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatLight) {
      return;
    }
    const ReLightMaterial* mat = static_cast<const ReLightMaterial*>(basemat);
    addBasicInformation(mat);
    matData["lightID"] = mat->getLightID();
    boost::any textureData;
    exportTextures(mat, textureData);
    result = matData;
  }
};

}

#endif
