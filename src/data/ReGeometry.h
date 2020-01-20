/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef REGEOMETRY_H
#define REGEOMETRY_H

#include <QSharedPointer>

namespace Reality {

struct ReTriangle_s {
  int a,b,c;
};
typedef int ReTriangle_i[3];

//! Our definition of a triangle. We can access it either as an array
//! or as a struct
union ReTriangle {
  ReTriangle_s s;
  ReTriangle_i a;
};

typedef QSharedPointer<ReTriangle> ReTrianglePtr;
typedef QList<ReTrianglePtr> ReTriangleList;

} // namespace

#endif
