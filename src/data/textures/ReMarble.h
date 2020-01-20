/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MARBLE_H
#define RE_MARBLE_H

#include "ReProceduralNoise.h"

namespace Reality {

/**
  Class: Marble

  3D
 */
class RE_LIB_ACCESS ReMarble : public ReProceduralNoise {
public:
  enum VeinQuality {
    SOFT,
    SHARP,
    SHARPER
  };

  enum VeinWave {
    SIN,
    SAWTOOTH,
    TRIANGLE
  };

private:
  float depth;
  float turbulence;
  VeinQuality veinQuality;
  VeinWave veinWave;
  bool softNoise;

public:
  // Constructor: ReMarble
  ReMarble(const QString name, ReTextureContainer* parentMat = 0);

  // Conversion ctor
  ReMarble( const ReTexturePtr srcTex );

  // Destructor: ReMarble
  ~ReMarble() {
  };

  virtual QString toString() {
    return "ReMarble texture";
  };

  /*
   Method: getVeinQuality
   */
  VeinQuality getVeinQuality() const;

  const QString getVeinQualityAsString() const;

  /*
   Method: setVeinQuality
   */
  void setVeinQuality( VeinQuality newVal );

  /*
   Method: getVeinWave
   */
  VeinWave getVeinWave() const;

  const QString getVeinWaveAsString() const;

  /*
   Method: setVeinWave
   */
  void setVeinWave( VeinWave newVal );

  /*
   Method: getDepth
   */
  float getDepth() const;
  /*
   Method: setDepth
   */
  void setDepth( float newVal );

  /*
   Method: getTurbulence
   */
  float getTurbulence() const;

  /*
   Method: setTurbulence
   */
  void setTurbulence( float newVal );

  /*
   Method: getSoftNoise
   */
  bool hasSoftNoise() const;

  /*
   Method: setSoftNoise
   */
  void setSoftNoise( bool newVal );

  void setNamedValue( const QString& name , const QVariant& value );
  const QVariant getNamedValue( const QString& name );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );
  
};

// Typedef: ReMarblePtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReMarble> ReMarblePtr;

} // namespace


#endif
