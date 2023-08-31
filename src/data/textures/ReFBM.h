/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_FBM_H
#define RE_FBM_H

#include "ReTexture.h"


namespace Reality {

/**
 * Implements the LuxRender FBM and Wrinkled textures, which have exactly the
 * same input.
 *
 * See also <ReTexture>.  
 */
class ReFBM : public ReTexture3D {
private:
  quint16 octaves;
  float roughness;

  // When this flag is enabled the output texture is of 
  // type "Wrinkled" instead of FBM
  bool wrinkled;

public:
  // Constructor: ReFBM
  ReFBM(const QString name, ReTextureContainer* parentMat = 0);

  // Conversion ctor
  ReFBM( const ReTexturePtr srcTex );

  // Destructor: ReFBM
  ~ReFBM() {
  };

  /*
   Method: getOctaves
   */
  quint16 getOctaves() const;
  /*
   Method: setOctaves
   */
  void setOctaves( quint16 newVal );
  
  /*
   Method: getRoughness
   */
  float getRoughness() const;

  /*
   Method: getWrinkled
   */
  bool isWrinkled() const;

  /*
   Method: setWrinkled
   */
  void setWrinkled( bool newVal );

  /*
   Method: setRoughness
   */
  void setRoughness( float newVal );

  void setNamedValue( const QString& name , const QVariant& value );
  const QVariant getNamedValue( const QString& name );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  virtual QString toString() {
    return "ReFBM texture";
  };
  
};

// Typedef: FBMPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReFBM> ReFBMPtr;


} // namespace

#endif
