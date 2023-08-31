/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef ReDSTools_H
#define ReDSTools_H

#include <dzmatrix3.h>

#include "ReMatrix.h"


namespace Reality {

/**
 * Converts a DzMatrix3 to a ReMatrix. The format is high-compatible,
 * the ReMatrix is simply a 4x4 matrix.
 */
inline void convertDzMatrix( const DzMatrix3& sm, ReMatrix& tm ) {
  tm.m[0][0] = sm[0][0];
  tm.m[0][1] = sm[0][1];
  tm.m[0][2] = sm[0][2];

  tm.m[1][0] = sm[1][0];
  tm.m[1][1] = sm[1][1];
  tm.m[1][2] = sm[1][2];

  tm.m[2][0] = sm[2][0];
  tm.m[2][1] = sm[2][1];
  tm.m[2][2] = sm[2][2];

  tm.m[3][0] = sm[3][0];
  tm.m[3][1] = sm[3][1];
  tm.m[3][2] = sm[3][2];
}

} // namespace

#endif
