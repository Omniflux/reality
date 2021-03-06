/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_LIGHTMATERIAL_IMPORTER_H
#define RE_QT_LIGHTMATERIAL_IMPORTER_H

#include "ReLightMaterial.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>
#include "ReSceneData.h"

/**
  Class: ReLightMaterialImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class RE_LIB_ACCESS ReQtLightMaterialImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReLightMaterialPtr mat = baseMat.staticCast<ReLightMaterial>();
    importBasicInformation(mat, data);
    mat->setLightID(data.value("lightID").toString());
    mat->setLight(RealitySceneData->getLight(mat->getLightID()));
  }

  void importFromClipboard( ReMaterialPtr mat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    
  };

};

}

#endif
