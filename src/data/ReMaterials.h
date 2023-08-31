/**
 * File: ReMaterials.h
 * Reality plug-in
 * Copyright 2010-2012 Pret-a-3D/Paolo Ciccone. All rights reserved
 */

#ifndef REMATERIALS_H
#define REMATERIALS_H

#include "reality_lib_export.h"

class QDataStream;
class QString;

namespace Reality {
  class ReGeometryObject;
  class ReMaterial;
  enum ReMaterialType;
}

namespace Reality {

class REALITY_LIB_EXPORT ReMaterialCreator {
public:
  static ReMaterial* createMaterial( ReMaterialType matType, 
                                     const ReGeometryObject* parent,
                                     const QString matName );

  static ReMaterial* deserialize( QDataStream& dataStream, const ReGeometryObject* parent );
};

}

#endif
