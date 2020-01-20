/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_JSON_MATERIAL_EXPORTER_H
#define RE_JSON_MATERIAL_EXPORTER_H

#include "exporters/ReMaterialExporter.h"
#include "exporters/qt/ReQtMaterialExporterFactory.h"
#include "qjson/src/serializer.h"

namespace Reality {

/**
 * This class exports all the materials to the JSOn format. Since 
 * it's kept general and it uses the QVariantMap as an intermediate format,
 * one class is all that we need to export any material to JSON.
 */
class RE_LIB_ACCESS ReJSONMaterialExporter : public ReMaterialExporter {
public:
  // Constructor: ReLuxMateriaExporter
  ReJSONMaterialExporter() {
  };
  // Destructor: ReJSONMateriaExporter
  virtual ~ReJSONMaterialExporter() {
  };

  void exportTextures( const ReMaterial* mat, 
                       boost::any& result, 
                       bool isForPreview = false ) {
    // Nothing
  }

  void exportMaterial( const ReMaterial* baseMat, boost::any& result ) {    
    QString str;
    boost::any qMatData;
    auto exporter = ReQtMaterialExporterFactory::getExporter(baseMat);
    exporter->exportMaterial(baseMat, qMatData);

    QJson::Serializer json;
    str = json.serialize(boost::any_cast<QVariantMap>(qMatData));
    result = str;
  }

};

}

#endif
