/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LIGHT_H
#define RE_LIGHT_H

#include <QSharedPointer>

#include "reality_lib_export.h"
#include "ReMatrix.h"


namespace Reality {

#define RE_IBL_LIGHT_ID "IBL"
  
enum ReLightType {
  MeshLight,
  SpotLight,
  SunLight,
  InfiniteLight,
  PointLight,
  IBLLight,
  UndefinedLight
};

/**
  All lights are defined in this class. There are small variations between a spotlight and
  a mesh light, for example. Such small variations are not worth the overhead required
  to convert from one type to the other and so all Reality lights are from one single
  class.

 */
class REALITY_LIB_EXPORT ReLight {
public:
  enum LightColorModel {
    Temperature,
    RGB,
    Preset
  };

  enum IBLType {
    LatLong, Angular
  };

protected:
  static QString ReLightTypeNames[UndefinedLight+1];
  static QString ReLightDefaultGroupNames[UndefinedLight+1];

  //! Introspection variable used to query what type of
  //! light is this.
  ReLightType type;

  //! Every light has a publicly visible name  
  QString name;

  //! The ID is a unique identifier that is different
  //! from the name. The ID is used to uniquely identify the light
  //! in the catalog. In this way we can have two or more lights 
  //! with the same name but different IDs, which makes it possible
  //! to have material-based lights with the same public name, which
  //! comes from materials with the same name, but originated by
  //! different objects. Using this startegy makes it possible to
  //! handle the lights correctly in that case.
  QString id;
  
  //! Declares if the light is on or off
  bool isOn;

  //! The brightness (gain in Lux) of the light
  float intensity;

  //! RGB color for the light
  QColor color;

  //! The name of the group that controls this light. Used to
  //! control lights dynamically whe rendering via Lux.
  QString lightGroup;
  //! Brightness of the sky light
  float skyIntensity;

  //! Color temperature of the light
  quint16 colorTemperature;
  //! ID of the preset used
  QString presetCode;
  //! Description of the preset used
  QString presetDesc;

  //! This variable specifies if we use color temperature, RGB or preset
  //! for the this light
  LightColorModel colorModel;

  //! The name of t aIES file to be used with the light
  QString iesFileName;

  // Variable: invertedNormals
  //   Used to invert the direction of ligh, for example with Light Domes.
  //   It defaults to false. Only for mesh lights.
  bool invertedNormals;
  float efficiency;
  float power;

  // The following properties are valid only for Spotlights
  float angle;
  float feather;

  //! Sun turbidity
  float sunTurbidity;
  //! Sky turbidity
  float skyTurbidity;
  //! Scale of the sun. Normal scale is 1.0
  union sunScaleOrGamma {
    float sunScale;
    float gamma;
  } extraData;

  bool newStyleSky;

  ReMatrix matrix;

  // IBL
  QString IBLMapFileName;
  IBLType IBLMapFormat;
  float IBLRotationAngle;

  //! Relevant only for mesh lights. If enabled the areas of the texture
  //! attached to the mesh light that are black will be transparent
  bool alphaChannel;

  //! True if this light has been converted from a material
  bool fromMaterial;

  //! Name of the file used for the texture/projector. Also used to store
  //! the name of the IBL preview map for IBL light
  QString imageFileName;
  
  //! Whether this light is an area light native of the host. For example, the
  //! Poser 11 area light.
  bool isHostAreaLightFlag;

private:

  //! Check if the light is a sun light and if so set the type
  //! acconrdingly.
  void testForSun();

public:

  // Constructor: ReLight
  ReLight( const QString name, const QString id );

  ReLight( const QString name, const QString id, ReLightType type );

  virtual ~ReLight() {
    
  }

  /*
   Method: getType
   */
  ReLightType getType() const;
  /*
   Method: setType
   */
  void setType( ReLightType newVal );

  /*
   Method: getGetTypeAsString
   */
  const QString getGetTypeAsString() const;

  /*
   Method: getIntensity
   */
  float getIntensity() const;
  /*
   Method: setIntensity
   */
  void setIntensity( float newVal );

  /*
   Method: getName
   */
  QString getName() const;

  /*
   Method: setName
   */
  void setName( QString newVal );  

  const QString getID() const;
  void  setID( const QString newID);

  virtual void setNamedValue( QString& valueName, QVariant& value );
  
  bool isLightOn() const;
  void turnOnOff( const bool onOff );

  /*
   Method: getLightGroup
   */
  const QString getLightGroup() const;
  /*
   Method: setLightGroup
   */
  void setLightGroup( QString newVal );

  /*
   Method: getColor
   */
  const QColor getColor() const;
  /*
   Method: setColor
   */
  void setColor( QColor newVal );

  /*
   Method: getColorTemperature
   */
  quint16 getColorTemperature() const;
  /*
   Method: setColorTemperature
   */
  void setColorTemperature( quint16 newVal );

  /*
   Method: getPresetCode
   */
  const QString getPresetCode() const;
  /*
   Method: setPresetCode
   */
  void setPresetCode( QString newVal );

  /*
   Method: getPresetDescription
   */
  const QString getPresetDescription() const;
  /*
   Method: setPresetDescription
   */
  void setPresetDescription( QString newVal );

  /*
   Method: getColorModel
   */
  LightColorModel getColorModel() const;
  /*
   Method: setColorModel
   */
  void setColorModel( LightColorModel newVal );
  
  /*
   Method: getPower
   */
  float getPower() const;
  /*
   Method: setPower
   */
  void setPower( float newVal );

  /*
   Method: getEfficiency
   */
  float getEfficiency() const;
  /*
   Method: setEfficiency
   */
  void setEfficiency( float newVal );

  /*
   Method: getInvertedNormals
   */
  bool getInvertedNormals() const;
  /*
   Method: setInvertedNormals
   */
  void setInvertedNormals( bool newVal );

  /*
   Method: getAngle
   */
  float getAngle() const;
  /*
   Method: setAngle
   */
  void setAngle( float newVal );

  /*
   Method: getFeather
   */
  float getFeather() const;
  /*
   Method: setFeather
   */
  void setFeather( float newVal );

  /*
   Method: getSunTurbidity
   */
  float getSunTurbidity() const;
  /*
   Method: setSunTurbidity
   */
  void setSunTurbidity( float newVal );

  /*
   Method: getSkyTurbidity
   */
  float getSkyTurbidity() const;
  /*
   Method: setSkyTurbidity
   */
  void setSkyTurbidity( float newVal );

  /*
   Method: getSkyIntensity
   */
  float getSkyIntensity() const;
  /*
   Method: setSkyIntensity
   */
  void setSkyIntensity( float newVal );

  /*
   Method: getSunScale
   */
  float getSunScale() const;
  /*
   Method: setSunScale
   */
  void setSunScale( float newVal );

  float getGamma() const;
  void setGamma( float newVal );

  float hasNewStyleSky() const;

  void setNewStyleSky( bool newVal );

  /*
   Method: getSourceMaterial
   */
  const QString getSourceMaterial() const;

  /*
   Method: setSourceMaterial
   */
  void setSourceMaterial( QString newVal );

  /**
   Implementation of the ReTextureContainer interface.
   */
  QString getTexture();

  /*
   Method: setTexture
   */
  void setTexture( const QString& newVal );

  /*
   Method: getIesFileName
   */
  const QString getIesFileName() const;
  
  /*
   Method: setIesFileName
   */
  void setIesFileName( QString newVal );

  //! Set the transform matrix of the light. The matrix is a list of
  //! 16 floats
  void setMatrix( const QVariantList& tm );

  //! Overloaded version
  void setMatrix( const ReMatrix& m );
  
  ReMatrix* getMatrix();

  /*
   * IBL
   */
  /*
   Method: getMapFormat
   */
  IBLType getIBLMapFormat() const;
  
  //! Set the format of the IBL light: either latlong or angular
  void setIBLMapFormat( IBLType newVal );
  //! Overloaded version to take the literal string for the type
  void setIBLMapFormat( const QString& newVal );

  /*
   Method: getRotationAngle
   */
  float getIBLRotationAngle() const;
  
  /*
   Method: setRotationAngle
   */
  void setIBLRotationAngle( const float newVal );
  
  const QString getIBLMapFileName() const;
  void setIBLMapFileName( QString newVal );

  void setAlphaChannel( bool newVal );
  bool hasAlphaChannel();

  void setFromMaterial( bool newVal );
  bool isFromMaterial();

  inline void setIsHostAreaLight( const bool flag ) {
    isHostAreaLightFlag = flag;
  }

  inline bool isHostAreaLight() {
    return isHostAreaLightFlag;
  }

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  //! Assignment operator. Used to copy the seettings of one light into another
  ReLight& operator =( ReLight& src );

  QString toString();

};

typedef QSharedPointer<ReLight> ReLightPtr;

} // namespace


#endif
