/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef QT_GEOMETRY_OBJECT_EXPORTER_H
#define QT_GEOMETRY_OBJECT_EXPORTER_H

#include <boost/any.hpp>
#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReAcsel.h"
#include "ReQtMaterialExporter.h"
#include "ReQtMaterialExporterFactory.h"

namespace Reality {
  class ReMaterial;
  typedef QSharedPointer<ReMaterial> ReMaterialPtr;
  typedef QHashIterator<QString, ReMaterialPtr> ReMaterialIterator;
}


namespace Reality {

/**
 * This exporter saves a \ref ReGeometryObject into a QVariantMap object
 */
class REALITY_LIB_EXPORT ReQtGeometryObjectExporter {

public:
  QVariantMap exportGeometryObject( ReGeometryObjectPtr obj ) {
    QVariantMap objData,
                materials,
                originalShaders;

    ReMaterialIterator mi(obj->getMaterials());
    auto acsel = ReAcsel::getInstance();
    QString objID = obj->getInternalName();
    // Scan the materials
    while( mi.hasNext() ) {
      mi.next();
      ReMaterialPtr mat = mi.value();
      // Serialization of the material
      auto matExporter = ReQtMaterialExporterFactory::getExporter(mat.data());
      boost::any exportedData;
      matExporter->exportMaterial(mat.data(), exportedData);
      QString matName = mat->getName();
      materials[matName] = boost::any_cast<QVariantMap>(exportedData);
      // Get the original shaders
      QString shaderCode; 
      acsel->getOriginalShader(objID, matName, shaderCode);
      originalShaders[matName] = shaderCode;
    }

    QVariantList lights;
    ReMaterialIterator li(*obj->getLights());
    while( li.hasNext() ) {
      li.next();
      lights.append(li.key());
    }

    objData["name"]           = obj->getName();
    objData["internalName"]   = obj->getInternalName();
    objData["geometryFile"]   = obj->getGeometryFileName();
    objData["isLight"]        = obj->isLight();
    objData["visible"]        = obj->isVisible();
    objData["instanceSource"] = obj->getInstanceSourceID();
    objData["materials"]      = materials;
    objData["lights"]         = lights;
    objData["origShaders"]    = originalShaders;

    return objData;
  };
};

}
#endif
