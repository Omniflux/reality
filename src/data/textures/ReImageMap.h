/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_IMAGEMAP_H
#define RE_IMAGEMAP_H

#include "Re2DTexture.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * A ImageMap defines the bit-mapped image texture. It is expected to 
 * be loaded from a image file from disk. See <ReTexture>.
 */
class REALITY_LIB_EXPORT ImageMap : public Re2DTexture {

protected:
  float gain;
  float gamma;
  QString fileName;
  RGBChannel rgbChannel;
  //! Flag to mark this map as a normal map
  bool normalMap;

  // Verifies if the file is one of the recognized formats of normal map
  void checkIfNormalMap();
  
public:
  // ImageMap(const QString name, ReMaterial* parentMat = 0, const QString fName = "") : 
  ImageMap( const QString name, 
            ReTextureContainer* parentMat = 0, 
            const QString fName = "",
            ReTextureDataType dType = color );

  // Copy ctor
  ImageMap( const ImageMap& t2 );

  // Conversion ctor
  ImageMap( const ReTexturePtr srcTex );

  virtual const QString getGUID();

  /**
   * Operators
   */
  bool operator ==( const ImageMap& t2) const;

  inline float getGain() const {
    return gain;
  }
  void setGain( float newVal ) {
    gain = newVal;
  }
  inline float getGamma() const {
    return gamma;
  }
  void setGamma( float newVal ) {
    gamma = newVal;
  }

  inline QString getFileName() const {
    return fileName;
  }
  void setFileName( QString newVal ) {
    fileName = newVal;
    checkIfNormalMap();
  }

  /**
    Returns the value of property normalMap
    */
  inline bool isNormalMap() const {
    return normalMap;
  };

  /**
   Sets the value of property normalMap
   */
  inline void setNormalMap( bool enabled ) {    
    normalMap = enabled;
    if (enabled) {
      setGamma(1.0);
    }
  };

  static bool checkIfNormalMap( const QString fName );

  /*
   Method: getRgbChannel
   */
  RGBChannel getRgbChannel() const;
  /*
   Method: setRgbChannel
   */
  void setRgbChannel( RGBChannel newVal );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  void setNamedValue( const QString& name, const QVariant& value );
  const QVariant getNamedValue( const QString& name );

  //! Copy all the properties from a compatible texture
  virtual void copyProperties( const ReTexturePtr srcTex );

  virtual QString toString();

};

// Typedef: ImageMapPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ImageMap> ImageMapPtr;


} // namespace

#endif
