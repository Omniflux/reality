/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_TEX_GRAYSCALE_H
#define RE_TEX_GRAYSCALE_H

#include "reality_lib_export.h"
#include "textures/Re2DTexture.h"
#include "textures/ReComplexTexture.h"


namespace Reality {

/**
  A texture that convert an image map to grayscale. Implemented by using the Mix texture,
  and by plugging the float version of the source texture into the amount and setting the first mix 
  value to black and the second mix value to white.

  The base texture is converted to Grayscale either by using the texture channel, if
  the texture is an Image Map, or by multiplying by a color constant, if the texture
  is of a different kind. This is done, of course, in the texture exporter. This
  texture is just a wrapper to drive the process.

  Although this is a Grayscale texture, it also defines a color multiplication between
  the base texture and a given color. This can be used, for example, to shift the 
  grayscale result before the final conversion. It can also be used to darken then 
  resulting texture. If, for example, we take the base texture, multiply it with a
  dark shader of gray and then extract one of its channels, we will affect the result
  beyond what is possible by simply extracting one of the channels.

  See also <ReTexture>.
  
 */
class REALITY_LIB_EXPORT ReGrayscale : public Re2DTexture, public ReComplexTexture
{

private:
  RGBChannel rgbChannel;
  // ReTexturePtr texture;
  QColor color;

  QStringList deps;

public:
  // Constructor: ReGrayscale
  explicit ReGrayscale( const QString name, ReTextureContainer* parentMat = 0 );

  explicit ReGrayscale( const QString name, 
                        ReTextureContainer* parentMat, 
                        const int rgbChannel );

  // Texture conversion ctor
  explicit ReGrayscale( const ReTexturePtr srcTex );

  explicit ReGrayscale(const QString name,
                       ReTextureContainer* parentMat, 
                       const int rgbChannel, 
                       const ReTexturePtr texture,
                       const QColor color = QColor(255,255,255));

  RGBChannel getRgbChannel() const;
  /**
   Method: setRgbChannel
   */
  void setRgbChannel( RGBChannel newVal );
 
  const QString getRGBChannelAsString() const;

  QString toString() {
    return QString("ReGrayscale texture: %1, channel: %2")
             .arg(channels["tex"]->getName())
             .arg(getRGBChannelAsString());
  };

  /*
   Method: getColor
   */
  const QColor getColor() const;

  /*
   Method: setColor
   */
  void setColor( QColor newVal );

  //! Returns the contained texture
  inline ReTexturePtr getTexture() const {
    return channels.value("tex");
  };
  /*
   Method: setTexture
   */
  void setTexture( ReTexturePtr newVal );

  const QVariant getNamedValue( const QString& name );
  void setNamedValue( const QString& name, const QVariant& value );

  void reparent( ReTextureContainer* parentMat );
  void replaceInnerTexture( const QString textureName, ReTexturePtr newTexture );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  QStringList getDependencies();
  
};

// Typedef: GrayscalePtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReGrayscale> ReGrayscalePtr;


} // namespace


#endif
