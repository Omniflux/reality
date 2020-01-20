/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_NULL_H
#define RE_NULL_H

#include "ReMaterial.h"

namespace Reality {

class RE_LIB_ACCESS ReNull: public ReMaterial {

public:

  explicit ReNull( const QString name, const ReGeometryObject* parent ) : ReMaterial(name, parent) 
  {
    type = originalType = MatNull;
  }

};

typedef QSharedPointer<ReNull> ReNullPtr;

} // namespace

#endif
