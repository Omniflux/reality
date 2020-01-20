/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef RE_QT_NULL_EXPORTER_H
#define RE_QT_NULL_EXPORTER_H

#include "ReNull.h"
#include "ReQtMaterialExporter.h"

namespace Reality {

class RE_LIB_ACCESS ReQtNullExporter : public ReQtMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {    
    matData.clear();
    if (basemat->getType() != MatNull) {
      return;
    }
    auto mat = static_cast<const ReNull*>(basemat);
    addBasicInformation(mat);
    result = matData;
  }
};

}

#endif
