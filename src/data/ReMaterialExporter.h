#ifndef REMATERIALEXPORTER_H
#define REMATERIALEXPORTER_H

#include "ReMaterials.h"
namespace Reality {

/**
  Class: ReMaterialExporter
 */
class RE_LIB_ACCESS ReMaterialExporter {

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
