/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_NULL_LUX_EXPORTER_H
#define RE_NULL_LUX_EXPORTER_H

#include "ReNull.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "reality_lib_export.h"

/*
  Class: ReNullLuxExporter
 */

namespace Reality {

class REALITY_LIB_EXPORT ReNullLuxExporter : public ReLuxMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatNull) {
      result = QString();
      return;
    }
    auto mat = static_cast<const ReNull*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();
    str = QString("# Mat: %1\n").arg(matName);

    str += QString("MakeNamedMaterial \"%1\" \"string type\" [\"null\"] \n").arg(matName);      
    result = str;
  }
};

}

#endif
