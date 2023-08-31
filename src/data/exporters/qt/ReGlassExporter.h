/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef RE_JSON_GLASS_EXPORTER_H
#define RE_JSON_GLASS_EXPORTER_H

#include <boost/any.hpp>
#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReGlass.h"
#include "ReMaterial.h"
#include "ReQtMaterialExporter.h"


namespace Reality {

/**
 * This class exports the Glass material to the QVariantMap format
 */
class REALITY_LIB_EXPORT ReQtGlassExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatGlass) {
      return;
    }
    const ReGlass* mat = static_cast<const ReGlass*>(basemat);
    QString matName = mat->getUniqueName();
    addBasicInformation(mat);
    matData["glassType"]         = mat->getGlassType();
    matData["uRoughness"]        = mat->getURoughness();
    matData["vRoughness"]        = mat->getVRoughness();
    matData["thinFilmIOR"]       = mat->getThinFilmIOR();
    matData["thinFilmThickness"] = mat->getThinFilmThickness();
    matData["IOR"]               = mat->getIOR();
    matData["IORLabel"]          = mat->getIORLabel();
    matData["dispersion"]        = mat->hasDispersion();
    matData["cauchyB"]           = mat->getCauchyB();

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
