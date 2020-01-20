/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __REHAIR_H__
#define __REHAIR_H__

#include "ReMaterials.h"

namespace Reality {

/*
 Class: Hair

 A material for human hair.
 */
class RE_LIB_ACCESS ReHair: public DisplaceableMaterial {

public:

    ReHair( const QString name, const ReGeometryObject* parent ) : 
      DisplaceableMaterial(name, parent) {
      type = MatHair;

    }

};

} // namespace

#endif
