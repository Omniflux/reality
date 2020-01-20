/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_JSON_MAT_EXPORTER_FACTORY_H
#define RE_JSON_MAT_EXPORTER_FACTORY_H

#include <QHash>
#include <QSharedPointer>
#include <boost/any.hpp>
 
#include "ReMaterials.h"
#include "ReJSONMaterialExporter.h"

namespace Reality {


typedef QSharedPointer<ReJSONMaterialExporter> ReJSONMaterialExporterPtr;

/**
 * JSON versions of the materials are simply obtained from using the QVariantMap
 * exporter and then converting it to JSON. There is therefore only one exporter
 * that works for every type of material. So this factory is always returning 
 * the same object
 */
class RE_LIB_ACCESS ReJSONMaterialExporterFactory {
private:
  static ReJSONMaterialExporterPtr exporter;
public:
  static ReJSONMaterialExporterPtr getExporter( const ReMaterial* mat ) {
    if (exporter.isNull()) {
      exporter = ReJSONMaterialExporterPtr( new ReJSONMaterialExporter() );
    }
    return exporter;
  };

  static void convertMaterialToJSON( const ReMaterial* mat, QString& matDef ) {
    auto matExporter = getExporter(mat);
    boost::any result;
    matExporter->exportMaterial(mat,result);
    matDef = boost::any_cast<QString>(result);    
  }
};

}
#endif
