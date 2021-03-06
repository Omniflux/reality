#ifndef RE_HAIR_LUX_EXPORTER_H
#define RE_HAIR_LUX_EXPORTER_H

#include "ReHair.h"
#include "exporters/lux/ReLuxMaterialExporter.h"

/*
  Class: ReHairLuxExporter
 */

namespace Reality {

class RE_LIB_ACCESS ReHairLuxExporter : public ReLuxMaterialExporter {

public:

  //! Export the material using the native exporter's format
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatHair) {
      return;
    }
    auto mat = static_cast<const ReHair*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();
    str = QString("# Mat: %1\n").arg(matName);
    result = str;
  }
};

}

#endif
