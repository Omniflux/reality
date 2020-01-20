/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_DISTORTED_NOISE_H
#define RE_DISTORTED_NOISE_H

#include "ReTexture.h"
#include "ReProceduralNoise.h"

namespace Reality {

/*
  Class: ReDistortedNoise

  See also <ReTexture>.
  3D
 */

class ReDistortedNoise : public ReProceduralNoise {

protected:
  NoiseDistortionType distortionType;
  float distortionAmount;

public:

  // Constructor: ReDistortedNoise
  ReDistortedNoise(const QString name, ReTextureContainer* parentMat = 0);

  // Texture conversion ctor
  ReDistortedNoise( const ReTexturePtr srcTex );

  // Destructor: ReDistortedNoise
  virtual ~ReDistortedNoise() {
  };

  virtual QString toString() {
    return "ReDistortedNoise texture";
  };

  /*
   Method: getDistortionType
   */
  inline NoiseDistortionType getDistortionType() const {
    return distortionType;
  };

  inline QString getDistortionTypeAsString() const {
    return lutLuxNoiseBasisNames[distortionType];
  };

  /*
   Method: setDistortionType
   */
  inline void setDistortionType( NoiseDistortionType newVal ) {
    distortionType = newVal;
  };

  /*
   Method: getDistortionAmount
   */
  inline float getDistortionAmount() const {
    return distortionAmount;
  };
  /*
   Method: setDistortionAmount
   */
  inline void setDistortionAmount( float newVal ) {
    distortionAmount = newVal;
  };

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  void setNamedValue( const QString& name, const QVariant& value );
  const QVariant getNamedValue( const QString& name );

};

// Typedef: ReDistortedNoisePtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReDistortedNoise> ReDistortedNoisePtr;


} // namespace


#endif
