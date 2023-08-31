/**
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_GEOMETRYOBJECT_IMPORTER_H
#define RE_QT_GEOMETRYOBJECT_IMPORTER_H

#include <QVariantMap>

#include "ReAcsel.h"
#include "ReGeometryObject.h"


namespace Reality {

/**
  Restores an object from the QVariantMap data
 */

class ReQtGeometryObjectImporter {

public:
  //! Constructor: ReQtGeometryObjectImporter
  ReQtGeometryObjectImporter() {
  };

  // Destructor: ReQtGeometryObjectImporter
  virtual ~ReQtGeometryObjectImporter() {
  };

  ReGeometryObjectPtr importGeometryObject( const QVariantMap& data ) {
    QString objID = data.value("internalName").toString();

    ReGeometryObjectPtr obj = ReGeometryObjectPtr( 
      new ReGeometryObject(data.value("name").toString(), 
                           objID, 
                           data.value("geometryFile").toString()));
    obj->setVisible(data.value("visible").toBool());
    obj->setInstanceSourceID(data.value("instanceSource","").toString());

    // Restore the materials
    QVariantMap materials = data.value("materials").toMap();
    QMapIterator<QString, QVariant> mi(materials);

    // Restore the original shaders
    QVariantMap origShaders = data.value("origShaders").toMap();
    auto acsel = ReAcsel::getInstance();

    while(mi.hasNext()) {
      mi.next();
      QVariantMap aMat = mi.value().toMap();
      QString matName = mi.key();
      obj->restoreMaterial(matName, aMat);
      acsel->storeOriginalShader(objID, matName, origShaders[matName].toString());
    }
    // Note that the lights array is redundant because the lights list is
    // recreated as a result of restoring materials of type ReLightMaterial
    return obj;
  }

};


} // namespace

#endif
