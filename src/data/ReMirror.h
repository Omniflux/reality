/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REMIRROR_H
#define REMIRROR_H

#include "reality_lib_export.h"
#include "ReModifiedMaterial.h"


namespace Reality {

class REALITY_LIB_EXPORT ReMirror: public ReModifiedMaterial {
protected:
  float filmIOR;
  float filmThickness;

public:

  explicit ReMirror( const QString name, const ReGeometryObject* parent );

  void fromMaterial( const ReMaterial* bm );

  /*
   Method: getFilmIOR
   */
  float getFilmIOR() const;

  /*
   Method: setFilmIOR
   */
  void setFilmIOR( float newVal );

  float getFilmThickness() const;

  /*
   Method: setFilmThickness
   */
  void setFilmThickness( float newVal );

  /*
   Method: getKr
   */
  inline ReTexturePtr getKr() const {
    return channels["Kr"];
  };
  /*
   Method: setKr
   */
  void setKr( ReTexturePtr newVal );

  void setNamedValue( const QString& vname, const QVariant& value );
  const QVariant getNamedValue( const QString& vname ) const;

  /**
   This is used to transmit the material between the host-side and GUI-side
   parts of Reality.
  */
  void serialize( QDataStream& dataStream ) const;

  /**
   Rebuilds the material from a data stream. Used to transmit the material between the 
   host-side and GUI-side parts of Reality.
   */
  void deserialize( QDataStream& dataStream );

};
typedef QSharedPointer<ReMirror> ReMirrorPtr;

} // namespace

#endif
