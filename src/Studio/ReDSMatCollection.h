/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2015. All rights reserved.    
 */

#ifndef DS_MAT_COLLECTION_H
#define DS_MAT_COLLECTION_H

#include <QHash>

#include "ReMaterial.h"


namespace Reality {

/**
 * DAZ Studio holds the materials used in a scene in a flat list where each
 * material is identified by a unique number. Each polygon int he scene has a
 * link to one of those materials. 
 *
 * Reality identifies materials based on the name and each material is stored
 * together with the object that owns it.
 *
 * This class implements a simple catalog that is keyd by the Studio material
 * ID and point to a material into the Reality catalog. Using this table
 * we can render the Studio objects correctly, including materials that 
 * result form geo-grafting.
 */

class ReDSMatCollection {
private:
  typedef QHash<int, ReMaterialPtr> FlatMaterialTable;

  FlatMaterialTable matTable;

public:
  //! Constructor
  ReDSMatCollection() {
  };

  //! Destructor
 ~ReDSMatCollection() {
  };
  
  inline void storeMaterial( const int matID, const ReMaterialPtr mat ) {
    matTable[matID] = mat;
  }

  inline ReMaterialPtr getMaterial( const int matID ) const {
    return matTable[matID];
  }

  inline void clear() {
    matTable.clear();
  }
};

} // namespace

#endif
