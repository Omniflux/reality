/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __REHAIR_H__
#define __REHAIR_H__

#include "reality_lib_export.h"
#include "ReModifiedMaterial.h"


namespace Reality {

/*
 Class: Hair

 A material for human hair.
 */
class REALITY_LIB_EXPORT ReHair: public ReModifiedMaterial {

public:

    ReHair( const QString name, const ReGeometryObject* parent ) : 
      ReModifiedMaterial(name, parent) {
      type = MatHair;

    }

};

} // namespace

#endif
