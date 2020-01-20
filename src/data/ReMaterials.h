/**
 * File: ReMaterials.h
 * Reality plug-in
 * Copyright 2010-2012 Pret-a-3D/Paolo Ciccone. All rights reserved
 */

#ifndef REMATERIALS_H
#define REMATERIALS_H

#include <QTextStream>

#include "ReDefs.h"
// #include "ReVolumes.h"

#include "ReMaterial.h"

namespace Reality {

class RE_LIB_ACCESS MaterialCreator {
public:
  static ReMaterial* createMaterial( ReMaterialType matType, 
                                     const ReGeometryObject* parent,
                                     const QString matName );

  static ReMaterial* deserialize( QDataStream& dataStream, const ReGeometryObject* parent );
};

}

#endif
