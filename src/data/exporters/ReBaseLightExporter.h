/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_BASE_LIGHT_EXPORTER_H
#define RE_BASE_LIGHT_EXPORTER_H

#include <QString>

#include "reality_lib_export.h"
#include "ReLight.h"


namespace Reality {

/*
  Class: ReBaseLightExporter

  This is the base class for the exporter of light fixtures.
 */

class REALITY_LIB_EXPORT ReBaseLightExporter {

public:
  // Constructor: ReBaseLightExporter
  ReBaseLightExporter() {
  };
  // Destructor: ReBaseLightExporter
  virtual ~ReBaseLightExporter() {
  };

  virtual QString exportLight( const ReLightPtr light ) = 0;
  
};


} // namespace


#endif
