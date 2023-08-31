/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLightExporter.h"

#include <QColor>
#include <QSettings>
#include <QStringBuilder>

#include "RealityBase.h"
#include "ReLight.h"
#include "ReMatrix.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReSceneResources.h"
#include "ReTexture.h"
#include "ReTools.h"
#include "exporters/ReLuxSceneExporter.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReImageMap.h"


namespace Reality {

// Singleton support data
ReLuxLightExporter* ReLuxLightExporter::instance = NULL;

// Singleton implementation
ReLuxLightExporter* ReLuxLightExporter::getInstance() {
  if (instance == NULL) {
    instance = new ReLuxLightExporter();
  }
  return instance;
}

QString ReLuxLightExporter::exportMeshLight( const ReLightPtr light ) {
  QString lightName = light->getName();
  QString str = QString(
    "# Mesh light %1\n"
    "LightGroup \"%2\"\n"
  )
  .arg(lightName)
  .arg(light->getLightGroup());

  QString lightTextures;

  QString lightImagemap = light->getTexture();  
  QString texName;
  if (lightImagemap == "") {
    texName = writeColorModelTexture(light, str);
  }
  else {
    // Add texture support
    ReTexturePtr ltex = ReTexturePtr( new ReImageMap(QString("%1::L").arg(lightName), 0, lightImagemap) );
    ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(ltex);
    lightTextures += exporter->exportTexture(ltex);
    texName = ltex->getUniqueName();
  }

  str += "AreaLightSource \"area\" ";
  str += QString(
           "\"float power\" [%1] \"float efficacy\" [%2] \"float gain\" [%3] "
         )
         .arg(light->getPower())
         .arg(light->getEfficiency())
         .arg(light->getIntensity());

  str += QString(" \"texture L\" [\"%1\"] ").arg(texName);

  QString iesFile = light->getIesFileName();
  if ((iesFile != "") && RealitySceneData->hasTextureCollection()) {
    iesFile = ReSceneResources::getInstance()->collectTexture(
                   iesFile, ReTextureSize::T_ORIGINAL
                 );
  }
  
  if (iesFile != "") {
#if defined(_WIN32)
    str += QString(" \"string iesname\" [\"%1\"] ").arg(iesFile.replace('\\', '/')); 
#else 
    str += QString(" \"string iesname\" [\"%1\"] ").arg(iesFile); 
#endif
  }

  str += "\n";
  return lightTextures % str;
}

QString ReLuxLightExporter::writeColorModelTexture( ReLightPtr light, QString& str ) {
  QString texName = QString("%1::L").arg(light->getName());
  auto colorModel = light->getColorModel();

  // August 2015, LuxCore does not support anything else but RGB
  // infinite lights.
  bool usingLuxCore = RealitySceneData->isOCLRenderingON() 
                      || RealitySceneData->cpuAccelerationEnabled();
  if (usingLuxCore && light->getType() == InfiniteLight) {
    colorModel = ReLight::RGB;
  }

  switch(colorModel) {
    case ReLight::Temperature: {
      str += QString(
               "Texture \"%1\" \"color\" \"blackbody\" \"float temperature\" [%2]\n"
             )
             .arg(texName)
             .arg(light->getColorTemperature());
      break;
    }
    case ReLight::RGB: {
      QColor clr = light->getColor();
      str += QString(
               "Texture \"%1\" \"color\" \"constant\" \"color value\" [%2 %3 %4]\n"
             )
             .arg(texName)
             .arg(clr.redF())
             .arg(clr.greenF())
             .arg(clr.blueF());
      break;
    }
    case ReLight::Preset: {
      str += QString(
               "Texture \"%1\" \"color\" \"lampspectrum\" \"string name\" [\"%2\"]\n"
             )
             .arg(texName)
             .arg(light->getPresetCode());
      break;
    }
  }
  return texName;
}


QString ReLuxLightExporter::exportSpotLight( const ReLightPtr light ) {
  QString str = QString("# Spotlight: %1\nAttributeBegin\n").arg(light->getName());
  auto appId = RealityBase::getRealityBase()->getHostAppID();
  const ReMatrix* matrix = light->getMatrix();

#if defined(_WIN32)
  QString lightImagemap = light->getTexture().replace('\\', '/');  
#else 
  QString lightImagemap = light->getTexture();  
#endif
  if ((lightImagemap != "") && RealitySceneData->hasTextureCollection()) {
    lightImagemap = ReSceneResources::getInstance()->collectTexture(
                   lightImagemap, ReTextureSize::T_ORIGINAL
                 );
  }

  bool isProjector = lightImagemap != "";
  str += QString("LightGroup \"%1\"\n").arg(light->getLightGroup());

  str += "TransformBegin\n";
  bool convertToMesh = RealityBase::getConfiguration()->value(RE_CFG_USE_REAL_SPOTS).toBool();

  QString iesFile = light->getIesFileName();
  // If and IES file has been selected we export this light as a mesh light because
  // that's the best way of controlling it. The shape of the mesh is discarded and
  // the light emits the cone as described in the IES file.
  if ( (iesFile != "" || convertToMesh) && !isProjector ) {
    str += ReLuxSceneExporter::getMatrixString(*matrix, appId);
    str += exportMeshLight(light);
    str += QString(
             "Shape \"mesh\" \"string name\" [\"%1\"] \"integer triindices\" ["
            "0 1 2"
            "]\n"
            "\"point P\" ["
            "0 0 0 "
            "0.0 0.0 0.01 "
            "0.01 0.0 0.0]\n"
           )
           .arg(light->getID());
  }
  else {
    // If there is an imagemap texture attached then we convert the light to a projector
    if (isProjector) {
      str += ReLuxSceneExporter::getMatrixString(*matrix, appId);
      str += QString("Rotate 180 1 0 0\n");
      str += QString("LightSource \"projection\" \"float gain\" [%1] "
                     "\"float importance\" [1.0] "
                     "\"string mapname\" [\"%2\"] "
                     "\"float fov\" [%3]\n")
               .arg(light->getIntensity())
               .arg(lightImagemap)
               .arg(light->getAngle()/2);
    }
    else {
      // Standard Spotlight
      QString texName = writeColorModelTexture(light, str);
      // Obtain the position and target of the light's matrix
      ReVector position;
      ReVector target;  

      matrix->getPosition(position);
      matrix->getTarget(target);
      // Convert to Lux's vector notation
      vectorToLux(position);
      vectorToLux(target);

      str += QString( "LightSource \"spot\" \"point from\" [%1 %2 %3]\n"
                      "  \"point to\" [%4 %5 %6]\n"
                      "  \"float coneangle\" [%7]\n"
                      "  \"float conedeltaangle\" [%8]\n"
                      "  \"texture L\" [\"%9\"]\n"
                      "  \"float gain\" [%10]\n"
                      "  \"float efficacy\" [%11]\n"
                      "  \"float power\" [%12] ")
               .arg(convertUnit(position.X, appId))
               .arg(convertUnit(position.Y, appId))
               .arg(convertUnit(position.Z, appId))
               .arg(convertUnit(target.X, appId))
               .arg(convertUnit(target.Y, appId))
               .arg(convertUnit(target.Z, appId))
               // Turn out that Lux works with the half angle. We add 1/2 of the edge 
               // feather just to give some more "room"         
               .arg((light->getAngle()+(light->getFeather()/2))/2)  
               .arg(light->getFeather())
               .arg(texName)
               .arg(light->getIntensity())
               .arg(light->getEfficiency())
               .arg(light->getPower());
    }
  }
  str += "\n"
         "TransformEnd\n"
         "AttributeEnd\n";
  return str;
}

QString ReLuxLightExporter::exportIBLLight( const ReLightPtr light ) {
  float rotationAngle = light->getIBLRotationAngle();
  QString str = "AttributeBegin\n";

  // Need to flip the map because Lux projects it flipped  
  str += QString("Scale -1 1 1 \n");
  if ( rotationAngle ) {
    // Rotation in Lux is the opposite of Poser and DS
    str += QString("Rotate %1 0 0 1\n").arg(-rotationAngle);
  }
  str += "LightGroup \"IBL\"\n"
         "LightSource \"infinitesample\" "; 

  QString mapFileName = light->getIBLMapFileName();
  if ((mapFileName != "") && RealitySceneData->hasTextureCollection()) {
    mapFileName = ReSceneResources::getInstance()->collectTexture(
                    mapFileName, ReTextureSize::T_ORIGINAL
                  );
  }

  if (mapFileName != "") {
    str += QString(
             "\"string mapname\" [\"%1\"] \"float gain\" [%2] "
             "\"string mapping\" [\"%3\"] \"float gamma\" [%4]\n"
           )
#if defined(_WIN32)
           .arg(mapFileName.replace('\\', '/'))
#else
           .arg(mapFileName)
#endif
           .arg(light->getIntensity()*IBL_GAIN_MULTIPLIER)
           .arg(light->getIBLMapFormat() == ReLight::LatLong ? "latlong" : "angular" )
           .arg(light->getGamma());
  }
  else {
    qreal r,g,b;
    light->getColor().getRgbF(&r, &g, &b);
    str += QString(
             "\"color L\" [%1 %2 %3] \"float gamma\" [1.0] \"float gain\" [%4]\n"
           )
           .arg(r).arg(g).arg(b)
           .arg(light->getIntensity()*IBL_GAIN_MULTIPLIER);
  }
  str += "AttributeEnd\n";
  return str;
}

QString ReLuxLightExporter::exportPointLight( const ReLightPtr light ) {
  QString str = QString("# Pointlight: %1\n"
                        "AttributeBegin\n").arg(light->getName());
  QString lightName = light->getName();
  auto appID = RealityBase::getRealityBase()->getHostAppID();

  QString lightTextures;
  QString lightImagemap = light->getTexture();  
  QString texName;
  if (lightImagemap == "") {
    texName = writeColorModelTexture(light, str);
  }
  else {
    // Add texture support
    ReTexturePtr ltex = ReTexturePtr( 
      new ReImageMap(QString("%1::L").arg(lightName), 0, lightImagemap)
    );
    ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(ltex);
    lightTextures += exporter->exportTexture(ltex);
    texName = ltex->getUniqueName();
  }

  str += QString("LightGroup \"%1\"\n").arg(light->getLightGroup());
  ReVector pos;
  light->getMatrix()->getPosition(pos);
  vectorToLux(pos);
  str += QString(
           "LightSource \"point\" \"point from\" [%1 %2 %3]"
           "\"texture L\" [\"%4\"] \"float gain\" [%5] "
           "\"float efficacy\" [%6] \"float power\" [%7] "
         )
         .arg(convertUnit(pos.X, appID))
         .arg(convertUnit(pos.Y, appID))
         .arg(convertUnit(pos.Z, appID))
         .arg(texName)
         .arg(light->getIntensity())
         .arg(light->getEfficiency())
         .arg(light->getPower());

  QString iesFile = light->getIesFileName();
  if ((iesFile != "") && RealitySceneData->hasTextureCollection()) {
    iesFile = ReSceneResources::getInstance()->collectTexture(
                   iesFile, ReTextureSize::T_ORIGINAL
                 );
  }

  if (iesFile != "") {
    str += QString(" \"string iesname\" [\"%1\"] ")
#if defined(_WIN32)
           .arg(iesFile);            
#else 
           .arg(iesFile.replace('\\', '/'));
#endif
  }

  str += "\n"
         "AttributeEnd\n";
  return lightTextures % str;
}


QString ReLuxLightExporter::exportSunlight( const ReLightPtr light ) {
  QString str;
  str += "# Sunlight\n"
         "LightGroup \"sun\"\n"
         "AttributeBegin\n";

  // The use of the inverse matrix has been taken from LuxBlend 
  // In that point the matrix elements used for the orientation are:
  //  0,2 -- 1,2 -- 2,2
  // Or, the third column of the matrix
  const ReMatrix* matrix = light->getMatrix();
  ReMatrix mi = matrix->inverse();

  str += QString(
           "LightSource \"sun\" \"float gain\" [%1] \"float turbidity\" [%2] "
           "\"float relsize\" [%3] \"vector sundir\" [%4 %5 %6]\n" 
         )
         .arg(light->getIntensity())
         .arg(light->getSunTurbidity())
         .arg(light->getSunScale())
         .arg( mi.m[0][2] )   // x
         .arg(-mi.m[2][2] )   // z
         .arg( mi.m[1][2] );  // y
  str += "AttributeEnd\n"
         "LightGroup \"sky\"\n"
         "AttributeBegin\n";

  QString skyName = "sky";
  if (light->hasNewStyleSky()) {
    skyName = "sky2";
  }
  str += QString(
           "LightSource \"%1\" \"float gain\" [%2] \"float turbidity\" [%3] "
           "\"vector sundir\" [%4 %5 %6]\n"
         )
         .arg(skyName)
         .arg(light->getSkyIntensity())
         .arg(light->getSkyTurbidity())
         .arg( mi.m[0][2] )  // x
         .arg(-mi.m[2][2] )  // z
         .arg( mi.m[1][2] ); // y
  str += "AttributeEnd\n";

  return str;
}

QString ReLuxLightExporter::exportInfiniteLight( const ReLightPtr light ) {
  bool convertToMesh = RealityBase::getConfiguration()->value(RE_CFG_CONVERT_INFINITE).toBool();

  QString str;
  str += "# InfiniteLight\n"
         "AttributeBegin\n";

  // The exporter for the meshlight already exports the light group
  if (!convertToMesh) {
    str += QString("LightGroup \"%1\"\n").arg(light->getLightGroup());
  }

  ReMatrix* matrix = light->getMatrix();
  HostAppID hostApp = RealityBase::getRealityBase()->getHostAppID();
  // Distant/Infinite lights don't have a location so we set the light to
  // be up in the sky
  matrix->m[3][0] = 0.0;
  matrix->m[3][1] = valueToHostApp(RE_INFINITE_LIGHT_HEIGHT, hostApp);
  matrix->m[3][2] = 0.0;
  
  str += "TransformBegin\n";
  str += ReLuxSceneExporter::getMatrixString( *matrix, hostApp );

  if (convertToMesh) {
    str += "Rotate 180 1 0 0 \n";
    str += exportMeshLight(light);
    str += QString(
             "Shape \"disk\" \"string name\" [\"%1\"] \"float radius\" [1.5]\n"
           )
           .arg(light->getID());    
  }
  else {
    QString texName = writeColorModelTexture(light, str);
    str += QString(
             "LightSource \"distant\" \"float gain\" [%1] \"texture L\" [\"%2\"] "
             " \"point from\" [0 0 0] \"point to\" [0 -1 0]\n" 
           )
           .arg(light->getIntensity())
           .arg(texName);
  }
  str += "TransformEnd\n"
         "AttributeEnd\n";
  return str;
}

QString ReLuxLightExporter::exportLight( const ReLightPtr light ) {
  exportStr = "";
  switch( light->getType() ) {
    case MeshLight:
      exportStr += exportMeshLight(light);
      break;
    case SpotLight:
      exportStr += exportSpotLight(light);
      break;
    case SunLight:
      exportStr += exportSunlight(light);
      break;
    case InfiniteLight:
      exportStr += exportInfiniteLight(light);
      break;
    case PointLight:
      exportStr += exportPointLight(light);
      break;
    case IBLLight:
      exportStr += exportIBLLight(light);
      break;
    case UndefinedLight:
      break;
  }

  return exportStr;
}


} // namespace
