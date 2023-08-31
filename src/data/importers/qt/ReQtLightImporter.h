/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_LIGHT_IMPORTER_H
#define RE_QT_LIGHT_IMPORTER_H

#include <QVariantMap>

#include "ReLight.h"


namespace Reality {

/*
  Class: ReQtLightImporter
 */

class ReQtLightImporter {

public:
  // Constructor: ReQtLightImporter
  ReQtLightImporter() {
  };
  // Destructor: ReQtLightImporter
 ~ReQtLightImporter() {
  };

  ReLightPtr importLight( const QVariantMap& data ) {
    QString name,id;

    name = data.value("name").toString();
    id = data.value("id").toString();
    ReLightPtr light = ReLightPtr( new ReLight( name, id ) );
    light->setType( static_cast<Reality::ReLightType>(data.value( "type" ).toInt()) );
    light->turnOnOff( data.value( "isOn" ).toBool() );
    light->setIntensity( data.value( "intensity" ).toFloat() );
    light->setColor( data.value( "color" ).toString() );
    light->setLightGroup( data.value( "lightGroup" ).toString() );
    light->setSkyIntensity( data.value( "skyIntensity" ).toFloat() );
    light->setColorTemperature( data.value( "colorTemperature" ).toInt() );
    light->setPresetCode( data.value( "presetCode" ).toString() );
    light->setPresetDescription( data.value( "presetDesc" ).toString() );
    light->setColorModel(static_cast<ReLight::LightColorModel>( data.value( "colorModel" ).toInt() ));
    light->setIesFileName( data.value( "iesFileName" ).toString() );
    light->setInvertedNormals( data.value( "invertedNormals" ).toBool() );
    light->setEfficiency( data.value( "efficiency" ).toFloat() );
    light->setPower( data.value( "power" ).toFloat() );
    light->setAngle( data.value( "angle" ).toFloat() );
    light->setFeather( data.value( "feather" ).toFloat() );
    light->setSunTurbidity( data.value( "sunTurbidity" ).toFloat() );
    light->setSkyTurbidity( data.value( "skyTurbidity" ).toFloat() );
    if (light->getType() == SunLight) {
      light->setSunScale( data.value( "sunScale" ).toFloat() );
    }
    else {
      light->setGamma( data.value( "gamma", 1.0 ).toFloat() );
    }
    light->setNewStyleSky( data.value( "newStyleSky" ).toBool() );
    light->setIBLMapFileName( data.value( "IBLMapFileName" ).toString() );
    light->setIBLMapFormat( static_cast<ReLight::IBLType>
      ( data.value( "IBLMapFormat" ).toInt() ) 
    );
    light->setIBLRotationAngle( data.value( "IBLRotationAngle" ).toFloat() );
    light->setAlphaChannel( data.value( "alphaChannel", false ).toBool() );
    light->setTexture( data.value( "texture", "" ).toString() );
    light->setFromMaterial( data.value( "fromMaterial", false ).toBool() );
    light->setMatrix( data.value( "matrix" ).toList() );
    light->setIsHostAreaLight( data.value( "isHostAreaLight", false ).toBool() );

    return light;
  }
};


} // namespace

#endif
