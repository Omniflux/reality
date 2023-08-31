/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/
#ifndef RE_MATRIX_H
#define RE_MATRIX_H

#include <cassert>
#include <cmath>
#include <QDataStream>
#include <QTextStream>

#include "reality_lib_export.h"
#include "ReDefs.h"


namespace Reality {

inline void sinCos( float& sinValue, float& cosValue, float angle ) {
  sinValue = sin(angle);
  cosValue = cos(angle);
}

typedef float ReMatrixData[4][4];

/**
  Simple container class for a 4x4 transformation matrix in row x column format.
  The matrix represents the transformation as:

                       X Y Z W
                       -------
                       x x x 0
                       y y y 0
                       z z z 0
 translation vector -> x y z 1
 */
class REALITY_LIB_EXPORT ReMatrix {

public:
  ReMatrixData m;

  enum ReAxis {
    XAxis, YAxis, ZAxis
  };

  // Constructor: ReMatrix
  ReMatrix() {
    m[0][0] = m[0][1] = m[0][2] = m[0][3] =
    m[1][0] = m[1][1] = m[1][2] = m[1][3] =
    m[2][0] = m[2][1] = m[2][2] = m[2][3] = 0;
    
    m[3][0] = m[3][1] = m[3][2] = m[3][3] = 1;
  };
  
  ReMatrix( const QVariantList& lm ) {
    setValues(lm);
  }

  ReMatrix ( 
    float m11, float m12, float m13, float m14,
    float m21, float m22, float m23, float m24,
    float m31, float m32, float m33, float m34,
    float m41, float m42, float m43, float m44
  ) 
  {
    m[0][0] = m11; m[0][1] = m12; m[0][2] = m13; m[0][3] = m14;
    m[1][0] = m21; m[1][1] = m22; m[1][2] = m23; m[1][3] = m24;
    m[2][0] = m31; m[2][1] = m32; m[2][2] = m33; m[2][3] = m34;
    m[3][0] = m41; m[3][1] = m42; m[3][2] = m43; m[3][3] = m44;
  }

  void setValues( const QVariantList& lm ) {
    QVariantList row = lm[0].toList();
    m[0][0] = row[0].toFloat();
    m[0][1] = row[1].toFloat();
    m[0][2] = row[2].toFloat();
    m[0][3] = row[3].toFloat();

    row = lm[1].toList();
    m[1][0] = row[0].toFloat();
    m[1][1] = row[1].toFloat();
    m[1][2] = row[2].toFloat();
    m[1][3] = row[3].toFloat();

    row = lm[2].toList();
    m[2][0] = row[0].toFloat();
    m[2][1] = row[1].toFloat();
    m[2][2] = row[2].toFloat();
    m[2][3] = row[3].toFloat();

    row = lm[3].toList();
    m[3][0] = row[0].toFloat();
    m[3][1] = row[1].toFloat();
    m[3][2] = row[2].toFloat();
    m[3][3] = row[3].toFloat();
  }

  void setValues( const ReMatrix& m2 ) {
    m[0][0] = m2.m[0][0];
    m[0][1] = m2.m[0][1];
    m[0][2] = m2.m[0][2];
    m[0][3] = m2.m[0][3];

    m[1][0] = m2.m[1][0];
    m[1][1] = m2.m[1][1];
    m[1][2] = m2.m[1][2];
    m[1][3] = m2.m[1][3];

    m[2][0] = m2.m[2][0];
    m[2][1] = m2.m[2][1];
    m[2][2] = m2.m[2][2];
    m[2][3] = m2.m[2][3];

    m[3][0] = m2.m[3][0];
    m[3][1] = m2.m[3][1];
    m[3][2] = m2.m[3][2];
    m[3][3] = m2.m[3][3];
  }

  // Destructor: ReMatrix
  ~ReMatrix() {
  };

  float* getData() {
    return m[0];
  }

  // Adapted from "3D Math"
  float determinant() const {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) +
           m[0][1] * (m[1][2] * m[2][0] - m[1][0] * m[2][2]) +
           m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
  }

  ReMatrix inverse() const {
    float det = determinant();
    assert(fabs(det) > 0.000001f);

    float oneOverDet = 1.0f / det;

    ReMatrix im;
    // Compute the inverse
    im.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * oneOverDet;
    im.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * oneOverDet;
    im.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * oneOverDet;

    im.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * oneOverDet;
    im.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * oneOverDet;
    im.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * oneOverDet;

    im.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * oneOverDet;
    im.m[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * oneOverDet;
    im.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * oneOverDet;

    // Compute the inverse of the translation portion
    im.m[3][0] = -(m[3][0] * im.m[0][0] + m[3][1] * im.m[1][0] + m[3][2] * im.m[2][0]);
    im.m[3][1] = -(m[3][0] * im.m[0][1] + m[3][1] * im.m[1][1] + m[3][2] * im.m[2][1]);
    im.m[3][0] = -(m[3][0] * im.m[0][2] + m[3][1] * im.m[1][2] + m[3][2] * im.m[2][2]);

    return im;
  }

  void rotate(ReAxis axis, float angle) {
    // Get the sin and cosine of the rotation angle
    float s,c;

    sinCos(s, c, angle);

    switch(axis) {
      case XAxis:
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = c;    m[1][2] = s;
        m[2][0] = 0.0f; m[2][1] = -s;   m[2][2] = c; 
        break;
      case YAxis:
        m[0][0] = c;    m[0][1] = 0.0f; m[0][2] = -s;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
        m[2][0] = s;    m[2][1] = 0.0f; m[2][2] = c; 
        break;
      case ZAxis:
        m[0][0] = c;    m[0][1] = s;    m[0][2] = 0.0f;
        m[1][0] = -s;   m[1][1] = c;    m[1][2] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; 
        break;
    }
  }

  friend ReMatrix operator*( const ReMatrix& m1, const ReMatrix& m2 );

  ReMatrix& operator =( const ReMatrix& m2 ) {
    setValues(m2);
    return *this;
  }

  //! Returns the position of the object managed by this matrix
  void getPosition( ReVector& pos ) const {
    pos.X = m[3][0];
    pos.Y = m[3][1];
    pos.Z = m[3][2];
  }

  //! When the matrix is used for a camera or light, this function
  //! returns the direction where the camera or light points to
  void getTarget( ReVector& target ) const {
    // Calculate the target point
    ReVector forwards;
    ReVector position;
    getPosition(position);
    forwards.X = -m[2][0];
    forwards.Y = -m[2][1];
    forwards.Z = -m[2][2];

    target.X = position.X + forwards.X;
    target.Y = position.Y + forwards.Y;
    target.Z = position.Z + forwards.Z;
  }

  //! Returns the value of the "up" vector
  void getUpVector( ReVector& up ) const {
    up.X =  m[1][0];
    up.Y =  m[1][1];
    up.Z =  m[1][2];    
  }

  void serialize( QDataStream& dataStream ){
    dataStream << m[0][0] << m[0][1] << m[0][2] << m[0][3]
               << m[1][0] << m[1][1] << m[1][2] << m[1][3]
               << m[2][0] << m[2][1] << m[2][2] << m[2][3]
               << m[3][0] << m[3][1] << m[3][2] << m[3][3];
  };

  void serialize( QTextStream& textStream ){
    textStream << m[0][0] << RE_DATA_SEPARATOR
               << m[0][1] << RE_DATA_SEPARATOR
               << m[0][2] << RE_DATA_SEPARATOR
               << m[0][3] << RE_DATA_SEPARATOR
               << m[1][0] << RE_DATA_SEPARATOR
               << m[1][1] << RE_DATA_SEPARATOR
               << m[1][2] << RE_DATA_SEPARATOR
               << m[1][3] << RE_DATA_SEPARATOR
               << m[2][0] << RE_DATA_SEPARATOR
               << m[2][1] << RE_DATA_SEPARATOR
               << m[2][2] << RE_DATA_SEPARATOR
               << m[2][3] << RE_DATA_SEPARATOR
               << m[3][0] << RE_DATA_SEPARATOR
               << m[3][1] << RE_DATA_SEPARATOR
               << m[3][2] << RE_DATA_SEPARATOR
               << m[3][3];
  };


  void deserialize( QDataStream& dataStream ){
    dataStream >> m[0][0] >> m[0][1] >> m[0][2] >> m[0][3]
               >> m[1][0] >> m[1][1] >> m[1][2] >> m[1][3]
               >> m[2][0] >> m[2][1] >> m[2][2] >> m[2][3]
               >> m[3][0] >> m[3][1] >> m[3][2] >> m[3][3];
  };


  const QString toString() const {
    return QString(
      "%1 %2 %3\n"
      "%4 %5 %6\n"
      "%7 %8 %9\n"
      "%10 %11 %12\n"
    )
    .arg(m[0][0])
    .arg(m[0][1])
    .arg(m[0][2])
    .arg(m[1][0])
    .arg(m[1][1])
    .arg(m[1][2])
    .arg(m[2][0])
    .arg(m[2][1])
    .arg(m[2][2])
    .arg(m[3][0])
    .arg(m[3][1])
    .arg(m[3][2]);
  }

  QVariantList toList() {
    QVariantList l;
    QVariantList r1;
    QVariantList r2;
    QVariantList r3;
    QVariantList r4;

    r1.append(m[0][0]);
    r1.append(m[0][1]);
    r1.append(m[0][2]);
    r1.append(m[0][3]);

    r2.append(m[1][0]);
    r2.append(m[1][1]);
    r2.append(m[1][2]);
    r2.append(m[1][3]);

    r3.append(m[2][0]);
    r3.append(m[2][1]);
    r3.append(m[2][2]);
    r3.append(m[2][3]);

    r4.append(m[3][0]);
    r4.append(m[3][1]);
    r4.append(m[3][2]);
    r4.append(m[3][3]);

    l.append(QVariant(r1));
    l.append(QVariant(r2));
    l.append(QVariant(r3));
    l.append(QVariant(r4));
    return l;
  }

};

inline ReMatrix operator*( const ReMatrix& m1, const ReMatrix& m2 ) {
  ReMatrix retVal;

  retVal.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0];
  retVal.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1];
  retVal.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2];

  retVal.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0];
  retVal.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1];
  retVal.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2];

  retVal.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0];
  retVal.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1];
  retVal.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2];

  retVal.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m2.m[3][0];
  retVal.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m2.m[3][1];
  retVal.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m2.m[3][2];
  return retVal;
}


} // namespace



#endif
