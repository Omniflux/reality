/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReDefs.h"
#include "ReMaterials.h"
#include "ReCloth.h"
#include "ReGlossy.h"
#include "ReSkin.h"
#include "ReMatte.h"
#include "ReGlass.h"
#include "ReWater.h"
#include "ReMetal.h"
#include "ReMirror.h"
#include "ReHair.h"
#include "ReVelvet.h"
#include "ReNull.h"
#include "ReLightMaterial.h"

namespace Reality {


ReMaterial* MaterialCreator::createMaterial( ReMaterialType matType,
                                             const ReGeometryObject* parent,
                                             const QString matName ) 
{
  switch(matType) {
    case MatCloth: {
      return new ReCloth(matName, parent);
    }
    case MatGlossy: {
      return new Glossy(matName, parent);
    }
    case MatSkin: {
      return new ReSkin(matName, parent);
    }
    case MatMatte: {
      return new ReMatte(matName, parent);
    }
    case MatGlass: {
      return new ReGlass(matName, parent);
    }
    case MatWater: {
      return new ReWater(matName, parent);
    }
    case MatMetal: {
      return new ReMetal(matName, parent);
    }
    case MatMirror: {
      return new ReMirror(matName, parent);
    }
    case MatVelvet: {
      return new ReVelvet(matName, parent);
    }
    case MatHair: {
      return new ReHair(matName, parent);
    }
    case MatLight:
      return new ReLightMaterial(matName, parent);
    case MatNull: {
      return new ReNull(matName, parent);
    }
    default:
      return NULL;
  }
}

ReMaterial* MaterialCreator::deserialize( QDataStream& dataStream, 
                                          const ReGeometryObject* parent ) 
{
  int matType;
  QString matName;

  int pos = dataStream.device()->pos(); // assume nothing
  dataStream >> matType >> matName;
  dataStream.device()->seek(pos);
  ReMaterial* newMat;

  newMat = createMaterial( static_cast<ReMaterialType>(matType), parent, matName );
  if (newMat) {
    newMat->deserialize(dataStream);
  }
  return newMat;
}

} // namespace
