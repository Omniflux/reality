/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_LIGHT_EXPORTER_H
#define QT_LIGHT_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReLight.h"


namespace Reality {

class REALITY_LIB_EXPORT ReQtLightExporter {

public:
  QVariantMap exportLight( const ReLightPtr light ) {
    QVariantMap lightData;
    lightData["type"]             = light->getType();
    lightData["name"]             = light->getName();
    lightData["id"]               = light->getID();
    lightData["isOn"]             = light->isLightOn();
    lightData["intensity"]        = light->getIntensity();
    lightData["color"]            = light->getColor().name();
    lightData["lightGroup"]       = light->getLightGroup();
    lightData["skyIntensity"]     = light->getSkyIntensity();
    lightData["colorTemperature"] = light->getColorTemperature();
    lightData["presetCode"]       = light->getPresetCode();
    lightData["presetDesc"]       = light->getPresetDescription();
    lightData["colorModel"]       = light->getColorModel();
    QString iesFileName = light->getIesFileName();

    lightData["iesFileName"]     = iesFileName.replace('\\','/');
    lightData["invertedNormals"] = light->getInvertedNormals();
    lightData["efficiency"]      = light->getEfficiency();
    lightData["power"]           = light->getPower();
    lightData["angle"]           = light->getAngle();
    lightData["feather"]         = light->getFeather();
    lightData["sunTurbidity"]    = light->getSunTurbidity();
    lightData["skyTurbidity"]    = light->getSkyTurbidity();
    // This is also used as Gamma for IBL lights
    if (light->getType() != SunLight) {        
      lightData["gamma"] = light->getGamma();
    }
    else {
      lightData["sunScale"] = light->getSunScale();
    }
    lightData["newStyleSky"]      = light->hasNewStyleSky();
    QString iblFileName           = light->getIBLMapFileName();
    lightData["IBLMapFileName"]   = iblFileName.replace('\\', '/');
    lightData["IBLMapFormat"]     = light->getIBLMapFormat();
    lightData["IBLRotationAngle"] = light->getIBLRotationAngle();
    lightData["alphaChannel"]     = light->hasAlphaChannel();
    lightData["fromMaterial"]     = light->isFromMaterial();
    lightData["texture"]          = light->getTexture().replace('\\', '/');
    lightData["matrix"]           = light->getMatrix()->toList();
    lightData["isHostAreaLight"]  = light->isHostAreaLight();
    return lightData;
  };
};

}
#endif
