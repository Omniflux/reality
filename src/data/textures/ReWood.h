/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_WOOD_H
#define RE_WOOD_H

#include "ReProceduralNoise.h"
#include "reality_lib_export.h"

namespace Reality {

/**
  Procedura Wood

  3D texture.
 */

class REALITY_LIB_EXPORT ReWood : public ReProceduralNoise {
public:
  enum VeinWave {
    SIN,
    SAWTOOTH,
    TRIANGLE
  };

  enum WoodPattern {
    RINGS,
    BANDS,
    BAND_NOISE,
    RING_NOISE
  };

  // LookUp Table (LUT) that converts the enum values to
  // strings meant to be displayed in the UI.
  static QString lutPatternsToStrings[RING_NOISE+1];

  // LookUp Table (LUT) that converts the enum values to
  // their string equivalent. We keep one single instance of 
  // this as it's used for other derived classes as well
  static QString lutPatternsToNames[RING_NOISE+1];

private:
  float turbulence;
  VeinWave veinWave;
  WoodPattern woodPattern;
  bool softNoise;

public:
  // Constructor: ReWood
  ReWood( const QString name, ReTextureContainer* parentMat = 0 );

  // Conversion ctor
  ReWood( const ReTexturePtr srcTex );

  // Destructor: ReWood
  ~ReWood() {
  };

  virtual QString toString() {
    return "ReWood texture";
  };

  /*
   Method: getVeinWave
   */
  VeinWave getVeinWave() const;

  const QString getVeinWaveAsString() const;

  /*
   Method: getWoodPattern
   */
  WoodPattern getWoodPattern() const;

  const QString getWoodPatternAsString() const;

  /*
   Method: setWoodPattern
   */
  void setWoodPattern( WoodPattern newVal );


  /*
   Method: setVeinWave
   */
  void setVeinWave( VeinWave newVal );

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

// Typedef: ReWoodPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReWood> ReWoodPtr;

} // namespace


#endif
