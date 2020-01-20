/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_COLOR_MATH_H
#define RE_COLOR_MATH_H

#include "textures/ReMath.h"
#include "textures/ReConstant.h"
#include "textures/ReComplexTexture.h"  

namespace Reality {

// Name of the channels used to store the sub-textures
#define CM_TEX1 "tex1"
#define CM_TEX2 "tex2"

/**
 * A texture that imposes a mathematical operation between the RGB values of two textures.
 * The operation can be one of add, subtract or multiply. See also <ReTexture> and <ReMath>.
 */
class ReColorMath : public Re2DTexture,
                    public ReComplexTexture
{
public:

  // Enum: functions
  //   the available functions of the pixels of the textures
  enum functions {
    add, subtract, multiply, none
  };

protected:
  QColor color1;
  QColor color2;
  
  functions function;

  // Dependency list
  QStringList deps;

public:

  ReColorMath( const QString name, ReTextureContainer* parentMat = 0 );

  ReColorMath( const QString name, 
               ReTextureContainer* parentMat,
               ReTexturePtr tex1, 
               ReTexturePtr tex2, 
               functions func );

  // Just a color and a texture. No second texture and no function.
  ReColorMath( const QString name, 
               ReTextureContainer* parentMat,
               QColor col1, 
               ReTexturePtr tex1 );

  // Texture conversion ctor
  ReColorMath( const ReTexturePtr srcTex );

  ~ReColorMath() {
    // Nothing
  };

  virtual void setTextureDataType( ReTextureDataType newVal );
  void reparent( ReTextureContainer* parentMat );

  QStringList getDependencies() {
    deps.clear();
    if (!channels[CM_TEX1].isNull()) {
      deps << channels[CM_TEX1]->getName();
    }
    if (!channels[CM_TEX2].isNull()) {
      deps << channels[CM_TEX2]->getName();
    }
    return deps;
  };

  virtual const QString getGUID() {
    // This texture name : GUID of tex1 : GUID of tex2 : amount1 : amount 2
    return QString("%1:%2:%3:%4:%5")
             .arg(getTypeAsString())
             .arg(channels[CM_TEX1].isNull() ? "" : channels[CM_TEX1]->getGUID())
             .arg(channels[CM_TEX2].isNull() ? "" : channels[CM_TEX2]->getGUID())
             .arg(color1.name())
             .arg(color2.name());
  };

  QString toString();

  inline virtual void setColor1( QColor clr1 ) {
    color1 = clr1;
  }

  inline virtual void setColor2( QColor clr2 ) {
    color2 = clr2;
  }

  inline const QColor& getColor1() const {
    return color1;
  }

  inline const QColor& getColor2() const {
    return color2;
  }

  inline virtual void setTexture1( ReTexturePtr tex1 ) {
    channels[CM_TEX1] = tex1;
  }

  inline ReTexturePtr getTexture1() {
    return channels[CM_TEX1];
  }

  inline virtual void setTexture2( ReTexturePtr tex2 ) {
    channels[CM_TEX2] = tex2;
  }

  inline ReTexturePtr getTexture2() {
    return channels[CM_TEX2];
  }

  inline void setFunction( const functions func ) {
    function = func;
  }

  inline functions getFunction() const {
    return function;
  }

  void setNamedValue( const QString& name, const QVariant& value );
  const QVariant getNamedValue( const QString& name );
  void replaceInnerTexture( const QString textureName, ReTexturePtr newTexture );

  void setTexture1( QColor& clr );
  void setTexture2( QColor& clr );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

};

//! Convenient definition of a shared pointer
typedef QSharedPointer<ReColorMath> ReColorMathPtr;


} // namespace

#endif
