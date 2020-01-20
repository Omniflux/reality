/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REVELVET_H
#define REVELVET_H

#include "ReDisplaceableMaterial.h"

namespace Reality {

class RE_LIB_ACCESS ReVelvet: public DisplaceableMaterial {

protected:
  float thickness;

public:

  explicit ReVelvet( const QString name, const ReGeometryObject* parent );

  void fromMaterial( const ReMaterial* bm );

  //! Returns the Kd channel
  inline ReTexturePtr getKd() const {
    return channels["Kd"];
  };

  /**
   Method: setKd
   */
  void setKd( ReTexturePtr newVal );

  /**
   Method: getThickness
   */
  inline float getThickness() const {
    return thickness;
  };
  /*
   Method: setThickness
   */
  void setThickness( float newVal );

  /**
   Sets the properties values using a parameter for the name
   */
  void setNamedValue( const QString& vname, const QVariant& value );
  /**
   Get the value of a propertie by name
   */
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

typedef QSharedPointer<ReVelvet> ReVelvetPtr;


} // namespace


#endif
