/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REMATTE_H
#define REMATTE_H

#include "ReMaterial.h"
#include "ReDisplaceableMaterial.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 Merges matte and matter translucent features in one.
 */
class REALITY_LIB_EXPORT ReMatte: public DisplaceableMaterial {

protected:
  float roughness;
  bool conserveEnergy;
  bool translucent;

public:

  explicit ReMatte( const QString name, const ReGeometryObject* parent );

  virtual void fromMaterial( const ReMaterial* bm );

  inline ReTexturePtr getKd() const{
    return channels["Kd"];
  };

  void setKd(const ReTexturePtr _Kd);

  //! Handles the Translucency texture
  inline ReTexturePtr getKt() const {
    return channels["Kt"];
  };
  void setKt(const ReTexturePtr _Kt);

  /*
   Method: serialize      
  */
  void serialize( QDataStream& dataStream ) const;

  void deserialize( QDataStream& dataStream );

  void setNamedValue( const QString& vname, const QVariant& value );
  const QVariant getNamedValue( const QString& vname ) const;

  /*
   Method: getRoughness
   */
  inline float getRoughness() const {
    return roughness;
  };
  /*
   Method: setRoughness
   */
  inline void setRoughness( float newVal ) {
    roughness = newVal;
  };

  /**
   Method: isTranslucent
   */
  inline bool isTranslucent() const {
    return translucent;
  };
  /**
   Method: setTranslucent
   */
  inline void setTranslucent( const bool newVal ) {
    translucent = newVal;
  };

  /**
   Method: conservesEnergy
   */
  inline bool conservesEnergy() const {
    return conserveEnergy;
  };
  /**
   Method: setConserveEnergy
   */
  inline void setConserveEnergy( const bool newVal ) {
    conserveEnergy = newVal;
  };
  
};

typedef QSharedPointer<ReMatte> ReMattePtr;

} // namespace

#endif
