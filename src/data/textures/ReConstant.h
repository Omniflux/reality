/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_CONSTANT_H
#define RE_CONSTANT_H

#include "textures/Re2DTexture.h"


namespace Reality {

/**
 * Implements a solid-color texture
 *  See also <ReTexture>.
 */
class ReConstant : public Re2DTexture {

private:
  // Used for color constants
  QColor color;

public:
  //! Constructor
  ReConstant(const QString name, 
             ReTextureContainer* parentMat = 0, 
             const QColor clr = RE_WHITE_COLOR);

  ReConstant( const QString name, ReTextureContainer* parentMat, const int value );

  //! Creates a float constant texture
  ReConstant( const QString name, ReTextureContainer* parentMat, const float value );

  //! Create a color texture with a given color and data type given in the last parameter
  ReConstant( const QString name, 
              ReTextureContainer* parentMat, 
              QColor clr, 
              ReTextureDataType dataType );

  // Copy ctor
  ReConstant( ReConstant& c2 );

  // Texture conversion ctor
  ReConstant( const ReTexturePtr srcTex );
  
  // Destructor: ReConstant
 ~ReConstant() ;

  inline QColor getColor() const {
    return color;    
  }
  
  inline void setColor(const QColor& newVal) {
    color = newVal;
  }

  virtual QString toString();
  
  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  virtual const QString getGUID() {
    return QString("%1::%2").arg(color.name()).arg(textureDataType);
  }

  void setNamedValue( const QString& name, const QVariant& value );

  const QVariant getNamedValue( const QString& name );

  bool operator ==( const ReConstant& c2 ) const {
    return color == c2.color && textureDataType == c2.textureDataType;
  }
};

//!   Convenient definition of a shared pointer
typedef QSharedPointer<ReConstant> ReConstantPtr;




} // namespace

#endif
