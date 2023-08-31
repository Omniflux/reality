/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_PROCEDURAL_NOISE_H
#define RE_PROCEDURAL_NOISE_H

#include "reality_lib_export.h"
#include "ReTexture.h"


namespace Reality {

class REALITY_LIB_EXPORT ReProceduralNoise : public ReTexture3D {

public:
  enum NoiseDistortionType {
    CELL_NOISE,
    VORONOI_CRACKLE,
    VORONOI_F1,
    VORONOI_F2,
    VORONOI_F2_F1,
    VORONOI_F3,
    VORONOI_F4,
    IMPROVED_PERLIN,
    ORIGINAL_PERLIN,
    BLENDER_ORIGINAL
  };

protected:
  float noiseSize;
  float brightness;
  float contrast;

  NoiseDistortionType noiseBasis;

public:

  // LookUp Table (LUT) that converts the enum values to
  // strings meant to be displayed in the UI.
  static QString lutTypesToStrings[BLENDER_ORIGINAL+1];

  // LookUp Table (LUT) that converts the enum values to
  // their string equivalent. We keep one single instance of 
  // this as it's used for other derived classes as well
  static QString lutLuxNoiseBasisNames[BLENDER_ORIGINAL+1];

  ReProceduralNoise(const QString name, ReTextureContainer* parentMat = 0);

  // Texture conversion ctor
  ReProceduralNoise( const ReTexturePtr srcTex );

  /*
   Method: getNoiseSize
   */
  inline float getNoiseSize() const {
    return noiseSize;
  };
  /*
   Method: setNoiseSize
   */
  inline void setNoiseSize( float newVal ) {
    noiseSize = newVal;
  };
  /*
   Method: getBrightness
   */
  inline float getBrightness() const {
    return brightness;
  };
  /*
   Method: setBrightness
   */
  inline void setBrightness( float newVal ) {
    brightness = newVal;
  };
  /*
   Method: getContrast
   */
  inline float getContrast() const {
    return contrast;
  };
  /*
   Method: setContrast
   */
  inline void setContrast( float newVal ) {
    contrast = newVal;
  };
  /*
   Method: getNoiseBasis
   */
  inline NoiseDistortionType getNoiseBasis() const {
    return noiseBasis;
  };

  /*
   Method: getNoiseBasis
   */
  inline QString getNoiseBasisAsString() const {
    return lutLuxNoiseBasisNames[noiseBasis];
  };

  /*
   Method: setNoiseBasis
   */
  inline void setNoiseBasis( NoiseDistortionType newVal ) {
    noiseBasis = newVal;
  };

  void setNamedValue( const QString& name, const QVariant& value );

  const QVariant getNamedValue( const QString& name );


  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  //! Copy all the properties from a compatible texture
  virtual void copyProperties( const ReTexturePtr srcTex );

};

typedef QSharedPointer<ReProceduralNoise> ReProceduralNoisePtr;

} // namespace

#endif
