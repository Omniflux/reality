/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RELIGHTSTAND_H
#define RELIGHTSTAND_H

#include "ReLight.h"

namespace Reality {

/**
  A simple Interface class to represent an object that holds a reference to a 
  mesh light. 

  A mesh light is a portion of a mesh that has a special material that is flagged to emit light. 

  We have two cases of mesh lights:

    - A geometry object that is completely dedicated to being a light. This is usually
      a square that has a name starting with "RealityLight"
    - A material of a standard obvject is converted to light by the user, using the
      special command in the Reality UI.

  The implementation that we use is to treat the base mesh like any other object
  and assign a special material to the portion that emits light. This special material 
  implements the <ReLightStand> interface and contains a link to a <ReLight> object.

  The <ReLight> object is the light proper and it can be edited in the Reality light
  editor. At the same time the <ReLight> object needs to hold a reference to the 
  object that contains it in cases the user needs to revert the mesh light
  to the original material.

 */

class RE_LIB_ACCESS ReLightStand {

public:
  // Constructor: ReLightStand
  ReLightStand() {
  };
  
  // Destructor: ReLightStand
  virtual ~ReLightStand() {
  };

  virtual ReLightPtr getLight() = 0;
};


} // namespace

#endif
