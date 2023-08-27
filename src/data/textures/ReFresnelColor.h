/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __RE_FRESNEL_H__
#define __RE_FRESNEL_H__

#include "ReTexture.h"
#include "ReTextureContainer.h"

namespace Reality {

/*
  Class: ReFresnelColor

  See also <ReTexture>.  
 */
class ReFresnelColor : public ReTexture {

protected:
  QColor color;
  ReTexturePtr tex;

  QStringList deps;

public:
  // Constructor: ReFresnelColor
  // ReFresnelColor(const QString name, ReMaterial* parentMat = 0) : 
  ReFresnelColor(const QString name, ReTextureContainer* parentMat = 0) : 
    ReTexture(name, parentMat)  {

    type = TexFresnelColor;
    textureDataType = fresnel;
  };

  // ReFresnelColor(const QString name, ReMaterial* parentMat, const QColor color) : 
  ReFresnelColor(const QString name, ReTextureContainer* parentMat, const QColor color) : 
    ReTexture(name, parentMat),
    color(color)  
  {

    textureDataType = fresnel;
    type = TexFresnelColor;
        
  };
  // Texture conversion ctor
  ReFresnelColor( const ReTexturePtr srcTex ) :
    ReTexture(srcTex)
  {
    type = TexFresnelColor;
    textureDataType = fresnel;
  }

  // ReFresnelColor(const QString name, ReMaterial* parentMat, const QColor color, const ReTexturePtr tex) : 
  ReFresnelColor(const QString name, ReTextureContainer* parentMat, const QColor color, const ReTexturePtr tex) : 
    ReTexture(name, parentMat),
    color(color),
    tex(tex)   {

    type = TexFresnelColor;
    textureDataType = fresnel;
    if (parent) {
      parent->addTextureToCatalog(tex);      
    }
  };

  // Destructor: ReFresnelColor
 ~ReFresnelColor() {
  };

  inline ReTexturePtr getTexture() {
    return tex;
  }

  inline QColor getColor() {
    return color;
  }

  // Fresnel textures cannot be changed to be of a different data type
  void setTextureDataType( ReTextureDataType /* newVal */ ) {

  }

  virtual QString toString();
  
  void reparent( ReTextureContainer* parentMat );

  QStringList getDependencies();

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

};

// Typedef: ReFresnelColorPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReFresnelColor> ReFresnelColorPtr;


} // namespace


#endif
