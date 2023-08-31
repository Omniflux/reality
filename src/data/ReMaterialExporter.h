#ifndef REMATERIALEXPORTER_H
#define REMATERIALEXPORTER_H

#include "reality_lib_export.h"

namespace Reality {
  class ReMaterial;
}


namespace Reality {

/**
  Class: ReMaterialExporter
 */
class REALITY_LIB_EXPORT ReMaterialExporter {

public:
  // Constructor: ReMaterialExporter
  ReMaterialExporter() {
  };

  // Destructor: ReMaterialExporter
 ~ReMaterialExporter() {
  };
  
  virtual const QString exportMaterial( const ReMaterial* mat ) const = 0;
};

}

#endif
