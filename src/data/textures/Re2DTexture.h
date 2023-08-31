/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_2DTEXTURE_H
#define RE_2DTEXTURE_H

#include "reality_lib_export.h"
#include "ReTexture.h"


namespace Reality {

class REALITY_LIB_EXPORT Re2DTexture : public ReTexture {

public:
  enum Mapping2D {
    UV,
    CYLINDRICAL,
    SPHERICAL,
    PLANAR
  };

protected:
  float uTile;
  float vTile;
  float uOffset;
  float vOffset;
  Mapping2D mapping;

public:

  // Re2DTexture( const QString name, ReMaterial* parentMat = 0 ) : 
  Re2DTexture( const QString name, ReTextureContainer* parentMat = 0 ) : 
    ReTexture(name, parentMat),
    uTile(1.0), 
    vTile(1.0), 
    uOffset(0.0), 
    vOffset(0.0),
    mapping(UV)
  {
     // Nothing
  };

  Re2DTexture( const Re2DTexture& t2 ) : 
    ReTexture(t2),
    uTile(t2.uTile), 
    vTile(t2.vTile), 
    uOffset(t2.uOffset), 
    vOffset(t2.vOffset),
    mapping(t2.mapping)
  {
     // Nothing
  };

  Re2DTexture( const ReTexturePtr srcTex ) : 
    ReTexture(srcTex)
  {
    QSharedPointer<Re2DTexture> src2 = srcTex.dynamicCast<Re2DTexture>();
    if (!src2.isNull()) {
      uTile   = src2->uTile;
      vTile   = src2->vTile;
      uOffset = src2->uOffset;
      vOffset = src2->vOffset;
      mapping = src2->mapping;
    }
    else {
      uTile   = 1.0;
      vTile   = 1.0;
      uOffset = 0.0;
      vOffset = 0.0;
      mapping = UV;
    }
  }

  ~Re2DTexture() {
    // Nothing
  }; 

  Re2DTexture& operator =( Re2DTexture& t2 ) {
    uTile   = t2.uTile;
    vTile   = t2.vTile;
    uOffset = t2.uOffset;
    vOffset = t2.vOffset;
    mapping = t2.mapping;
    return *this;
  }


  inline float getUTile() const  {
    return uTile;
  }
  inline void setUTile( float newVal ) {
    uTile = newVal;
  }

  inline float getVTile() const {
    return vTile;
  }
  inline void setVTile( float newVal ) {
    vTile = newVal;
  }

  inline float getUOffset() const {
    return uOffset;
  }
  inline void setUOffset( float newVal ) {
    uOffset = newVal;
  }

  inline float getVOffset() const {
    return vOffset;
  }
  inline void setVOffset( float newVal ) {
    vOffset = newVal;
  }

  inline Mapping2D getMapping() {
    return mapping;
  }
  inline void setMapping( Mapping2D newVal ) {
    mapping = newVal;
  }

  virtual void serialize( QDataStream& dataStream );

  virtual void deserialize( QDataStream& dataStream );

  void setNamedValue( const QString& name, const QVariant& value );

  const QVariant getNamedValue( const QString& name ) ;

  //! Copy all the properties from a compatible texture
  virtual void copyProperties( const ReTexturePtr srcTex );

};

typedef QSharedPointer<Re2DTexture> Re2DTexturePtr;

} // namespace


#endif
