/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REMETAL_H
#define REMETAL_H

#include "reality_lib_export.h"
#include "ReDisplaceableMaterial.h"


namespace Reality {

#define RE_METAL_DEFAULT_POLISH 9500
  
class REALITY_LIB_EXPORT ReMetal: public ReModifiedMaterial {

public:
  enum MetalType {
    Aluminum,
    Chrome,
    Cobalt,
    Copper,
    Gold,
    Lead,
    Mercury,
    Nickel,
    Platinum,
    Silver,
    Titanium,
    Zinc,
    Custom
  };

protected:
  MetalType metalType;
  // ReTexturePtr Kr;
  float hPolish, vPolish;

public:

  explicit ReMetal( const QString name, const ReGeometryObject* parent );

  virtual void fromMaterial( const ReMaterial* bm );
  
  /*
   Method: getKr
   */
  inline ReTexturePtr getKr() const {
    return channels["Kr"];
  };
  /*
   Method: setKr
   */
  void setKr( ReTexturePtr newVal ) ;

  /*
   Method: getMetalType
   */
  MetalType getMetalType() const;
  /*
   Method: setMetalType
   */
  void setMetalType( MetalType newVal );
  
  /*
   Method: getHPolish
   */
  float getHPolish() const;
  /*
   Method: setHPolish
   */
  void setHPolish( float newVal );
  
  /*
   Method: getVPolish
   */
  float getVPolish() const;
  /*
   Method: setVPolish
   */
  void setVPolish( float newVal );

  void setNamedValue( const QString& name, const QVariant& value );
  const QVariant getNamedValue( const QString& name ) const;

  void serialize( QDataStream& dataStream ) const;

  /*
   Method: deserialize

   Rebuilds the material from a data stream. Used to transmit the material between the 
   host-side and GUI-side parts of Reality.
   */
  void deserialize( QDataStream& dataStream );

};

typedef QSharedPointer<ReMetal> ReMetalPtr;

} // namespace

#endif
