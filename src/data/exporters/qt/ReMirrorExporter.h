/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MIRROR_EXPORTER_H
#define RE_QT_MIRROR_EXPORTER_H

#include "ReMirror.h"
#include "ReQtMaterialExporter.h"

/*
  Class: ReMirrorExporter
 */

namespace Reality {

class RE_LIB_ACCESS ReQtMirrorExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatMirror) {
      return;
    }
    auto mat = static_cast<const ReMirror*>(basemat);
    addBasicInformation(mat);
    matData["filmIOR"]       = mat->getFilmIOR();
    matData["filmThickness"] = mat->getFilmThickness();

    exportAlphaChannel(mat);
    exportModifiers(mat);
    boost::any textureData;
    exportTextures(mat, textureData);
    // The value returned by exportTextures() is a map containing
    // the definition of the textures and the channels. We need
    // to inject those two sets into the matData map
    try {
      QVariantMap tmp = boost::any_cast<QVariantMap>(textureData);
      matData["textures"] = tmp.value("textures");
      matData["channels"] = tmp.value("channels");
      result = matData;
    }
    catch( const boost::bad_any_cast& e ) {
      RE_LOG_INFO() << "Exception " << e.what();
    }
  }
};

}

#endif
