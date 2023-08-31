/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RECONSTANT_H
#define RECONSTANT_H

#include "textures/ReConstant.h"

#include "ReTools.h"
#include "textures/ReColorMath.h"

namespace Reality {

ReConstant::ReConstant( const QString name, 
                        ReTextureContainer* parentMat, 
                        const QColor clr ) :  
   Re2DTexture(name, parentMat), 
   color(clr) 
{
   type = TexConstant;
   textureDataType = ReTexture::color;
};

ReConstant::ReConstant( const QString name, 
                        ReTextureContainer* parentMat, 
                        QColor clr, 
                        ReTextureDataType dataType ): 
   Re2DTexture(name, parentMat),
   color(clr) 
{
   type = TexConstant;
   textureDataType = dataType;
   // Correction for colors that are either pure black or pure white
   if (isColorPureBlack(color)) {
     color = RE_BLACK_COLOR;
   }
   else if( isPureWhite(color) ) {
     color = RE_WHITE_COLOR;
   }
};



ReConstant::ReConstant( const QString name, 
                        ReTextureContainer* parentMat, 
                        const int value ): 
   Re2DTexture(name, parentMat), 
   color(QColor(value, value, value)) 
{
   type = TexConstant;
   textureDataType = numeric;
   // Correction for colors that are either pure black or pure white
   if (isColorPureBlack(color)) {
     color = RE_BLACK_COLOR;
   }
   else if( isPureWhite(color) ) {
     color = RE_WHITE_COLOR;
   }
};

ReConstant::ReConstant( const QString name, 
                        ReTextureContainer* parentMat, 
                        const float value ): 
   Re2DTexture(name, parentMat)
{
   color = QColor::fromRgbF(value, value, value);
   type = TexConstant;
   textureDataType = numeric;
   // Correction for colors that are either pure black or pure white
   if (isColorPureBlack(color)) {
     color = RE_BLACK_COLOR;
   }
   else if( isPureWhite(color) ) {
     color = RE_WHITE_COLOR;
   }
};


// Copy ctor
ReConstant::ReConstant( ReConstant& c2 ) : 
  Re2DTexture(c2.name, c2.parent) {
  color = c2.color;
  type = TexConstant;
};


// Texture conversion ctor
ReConstant::ReConstant( const ReTexturePtr srcTex ) :
  Re2DTexture(srcTex)
{
  type = TexConstant;
  // Pseudo copy ctor for texture conversion and preservation.
  // When the user converts a texture some target textures, like
  // Color Math and ReMixTexture preserve the original texture 
  // as the imput for the first texture. This requires that we have
  // a copy ctor for all texture but copy ctors are not matched
  // for objects allocated on the heap.
  switch ( srcTex->getType() ) {
    case TexConstant: {
      color = srcTex.staticCast<ReConstant>()->color;
      break;
    }
    case TexColorMath: {
      // Copy the color in Texture A
      auto tex1 = srcTex.staticCast<ReColorMath>()->getTexture1();
      if (tex1->getType() == TexConstant) {
        color = tex1.staticCast<ReConstant>()->getColor();
      }
      else {
        color = srcTex.staticCast<ReColorMath>()->getColor1();
      }
      break;
    }
    default:
      break;
  }
}

// Destructor: Constant
ReConstant::~ReConstant() {
};


QString ReConstant::toString(){
  return QString("ReConstant(%1) - %2").arg(name).arg(color.name());
};


void ReConstant::serialize( QDataStream& dataStream ) {
  Re2DTexture::serialize(dataStream);
  dataStream << color;
};


void ReConstant::deserialize( QDataStream& dataStream ) {
  Re2DTexture::deserialize(dataStream);
  dataStream >> color;
};



void ReConstant::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "color") {
    setColor(value.value<QColor>());
  }
}

const QVariant ReConstant::getNamedValue( const QString& name ) {
  if (name == "color") {
    return QVariant(getColor());
  }
  return "";
}

} //namespace

#endif
