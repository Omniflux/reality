/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLight.h"
#include "ReTools.h"
#include "ReTextureCreator.h"
#include "RealityBase.h"
#include "ReSceneDataGlobal.h"

using namespace Reality;

QString ReLight::ReLightTypeNames[UndefinedLight+1] = { 
  QString("Mesh Light"),  
  QString("Spot Light"),
  QString("Sun"),
  QString("Infinite Light"),
  QString("Point Light"),
  QString("IBL"),
  QString("Undefined")
};

QString ReLight::ReLightDefaultGroupNames[UndefinedLight+1] = { 
  QString(QObject::tr("Mesh Lights")),
  QString(QObject::tr("Spot Lights")),
  QString(QObject::tr("Sun Light")),
  QString(QObject::tr("Infinite Lights")),
  QString(QObject::tr("Point Lights")),
  QString(QObject::tr("IBL")),
  QString(QObject::tr("Reality Lights"))
};


/*
 * ReLight
 */
ReLight::ReLight( const QString name, const QString id ) : 
  type(UndefinedLight), 
  name(name), 
  id(id),
  isOn(true),
  intensity(1.0),
  color(QColor(255,255,255)),
  lightGroup(QObject::tr("Reality Lights")),
  skyIntensity(1.0),
  colorTemperature(6500),
  presetCode("Day light"),
  presetDesc(QObject::tr("White light|The light of the sun")),
  colorModel(Temperature),
  invertedNormals(false),
  efficiency(17),
  power(100),
  angle(60),
  feather(5),
  sunTurbidity(2.2f),
  skyTurbidity(2.2f),
  newStyleSky(false),
  IBLMapFormat(LatLong),
  IBLRotationAngle(0),
  alphaChannel(false),
  fromMaterial(false),
  isHostAreaLightFlag(false)
{
  extraData.sunScale = 1.0;
  testForSun();
};


ReLight::ReLight( const QString name, const QString id, ReLightType type ) :
  type(type), 
  name(name), 
  id(id),
  isOn(true),
  intensity(1.0),
  color(QColor(255,255,255)),
  lightGroup(QObject::tr("Reality Lights")),

  skyIntensity(1.0),
  // lightGroup(QObject::tr("Reality Lights")),
  colorTemperature(6500),
  presetCode("Day light"),
  presetDesc(QObject::tr("White light|The light of the sun")),
  colorModel(Temperature),
  invertedNormals(false),
  efficiency(17),
  power(100),
  angle(60),
  feather(5),
  sunTurbidity(2.2f),
  skyTurbidity(2.2f),
  newStyleSky(false),
  IBLMapFormat(LatLong),
  IBLRotationAngle(0),
  alphaChannel(false),
  fromMaterial(false),
  isHostAreaLightFlag(false)
{
  extraData.sunScale = 1.0;

  lightGroup = ReLightDefaultGroupNames[type];
  testForSun();
  // Distant/Infinite lights look so ugly in Lux that it is
  // better to turn them off automatically. If the user really
  // wants she can turn them on manually.
  if (type == InfiniteLight) {
    isOn = false;
  }
}

ReLight& ReLight::operator =( ReLight& src ) {
  type               = src.type;
  name               = src.name; 
  id                 = src.id;
  isOn               = src.isOn;
  intensity          = src.intensity;
  color              = src.color;
  lightGroup         = src.lightGroup;  
  skyIntensity       = src.skyIntensity;
  colorTemperature   = src.colorTemperature;
  presetCode         = src.presetCode;
  presetDesc         = src.presetDesc;
  colorModel         = src.colorModel;
  iesFileName        = src.iesFileName;
  invertedNormals    = src.invertedNormals;
  efficiency         = src.efficiency;
  power              = src.power;
  angle              = src.angle;
  feather            = src.feather;
  sunTurbidity       = src.sunTurbidity;
  skyTurbidity       = src.skyTurbidity;
  extraData.sunScale = src.extraData.sunScale;
  newStyleSky        = src.newStyleSky;
  matrix             = src.matrix;
  IBLMapFileName     = src.IBLMapFileName;
  IBLMapFormat       = src.IBLMapFormat;
  IBLRotationAngle   = src.IBLRotationAngle;
  alphaChannel       = src.alphaChannel;
  fromMaterial       = src.fromMaterial;
  imageFileName      = src.imageFileName;
  isHostAreaLightFlag = src.isHostAreaLightFlag;
  return *this;
}

ReLightType ReLight::getType() const {
  return type;
};

void ReLight::setType( ReLightType newVal ) {
  // We can't change the light type of the Sun
  if (type == SunLight) {
    return;
  }
  
  bool hasDefaultGroup = lightGroup == ReLightDefaultGroupNames[type];
  type = newVal;
  // If the light had the default group name then assign a new
  // default group name.
  if (hasDefaultGroup) {
    lightGroup = ReLightDefaultGroupNames[type];
  }
};


const QString ReLight::getGetTypeAsString() const {
  return ReLightTypeNames[type];
};

float ReLight::getIntensity() const {
  return intensity;
};

void ReLight::setIntensity( float newVal ) {
  intensity = newVal;
};

QString ReLight::getName() const {
  return name;
};

void ReLight::setName( QString newVal ) {
  name = newVal;
  if (name == "Sun") {
    setType(SunLight);
  }
};

const QString ReLight::getID() const {
  return id;
};

void ReLight::setID( QString newVal ) {
  id = newVal;
};

bool ReLight::isLightOn() const {
  return isOn;
}

void ReLight::turnOnOff( const bool onOff ) {
  isOn = onOff;
}

const QString ReLight::getLightGroup() const {
  return lightGroup;
};
void ReLight::setLightGroup( QString newVal ) {
  lightGroup = newVal;
};


const QColor ReLight::getColor() const {
  return color;
};
void ReLight::setColor( QColor newVal ) {
  color = newVal;
};

void ReLight::setFromMaterial( bool newVal ) {
  fromMaterial = newVal;
};

bool ReLight::isFromMaterial() {
  return fromMaterial;
};

void ReLight::testForSun() {
  if (type == InfiniteLight && name.toLower().trimmed() == "sun") {
    type = SunLight;
    lightGroup = ReLightDefaultGroupNames[SunLight];
  }
}

void ReLight::setNamedValue( QString& valueName, QVariant& value ) {
  if (valueName == "intensity") { 
    intensity = value.toFloat();
  }
  else if (valueName == "name") {
    name = value.toString();
  }
  else if (valueName == "id") {
    id = value.toString();
  }
  else if (valueName == "isOn") {
    isOn = value.toBool();
  }
  else if (valueName == "lightGroup") {
    lightGroup = value.toString();
  }
  else if (valueName == "color") {
    color = value.value<QColor>();
  }
  else if (valueName == "colorTemperature") {
    colorTemperature = value.toInt();
  }
  else if (valueName == "presetCode") {
    presetCode = value.toString();
  }
  else if (valueName == "presetDescription") {
    presetDesc = value.toString();
  }
  else if (valueName == "colorModel") {
    colorModel = static_cast<LightColorModel>(value.toInt());
  }
  else if (valueName == "texture") {
    imageFileName = value.toString();
  }
  else if (valueName == "iesFileName") {
    iesFileName = value.toString();
  }
  else if (valueName == "invertedNormals") {
    invertedNormals = value.toBool();
  }
  else if (valueName == "efficiency") {
    efficiency = value.toFloat();
  }
  else if (valueName == "power") {
    power = value.toFloat();
  }
  else if (valueName == "angle") {
    angle = value.toFloat();
  }
  else if (valueName == "feather") {
    feather = value.toFloat();
  }
  else if (valueName == "sunScale") {
    extraData.sunScale = value.toFloat();
  }
  else if (valueName == "gamma") {
    extraData.gamma = value.toFloat();
  }
  else if (valueName == "sunTurbidity") {
    sunTurbidity = value.toFloat();
  }
  else if (valueName == "skyIntensity") {
    skyIntensity = value.toFloat();
  }
  else if (valueName == "skyTurbidity") {
    skyTurbidity = value.toFloat();
  }
  else if (valueName == "newStyleSky") {
    newStyleSky = value.toBool();
  }
  else if (valueName == "mapFileName") {
    setIBLMapFileName(value.toString());
  }
  else if (valueName == "mapFormat") {
    IBLMapFormat = static_cast<IBLType>(value.toInt());
  }
  else if (valueName == "rotationAngle") {
    IBLRotationAngle =  value.toFloat();
  }
  else if (valueName == "alphaChannel") {
    alphaChannel = value.toBool();
  }
  else if (valueName == "fromMaterial") {
    fromMaterial = value.toBool();
  }

}

quint16 ReLight::getColorTemperature() const {
  return colorTemperature;
};
void ReLight::setColorTemperature( quint16 newVal ) {
  colorTemperature = newVal;
};


const QString ReLight::getPresetCode() const {
  return presetCode;
};
void ReLight::setPresetCode( QString newVal ) {
  presetCode = newVal;
};

const QString ReLight::getPresetDescription() const {
  return presetDesc;
};
void ReLight::setPresetDescription( QString newVal ) {
  presetDesc = newVal;
};

ReLight::LightColorModel ReLight::getColorModel() const {
  return colorModel;
};

void ReLight::setColorModel( LightColorModel newVal ) {
  colorModel = newVal;
};

float ReLight::getPower() const {
  return power;
};
void ReLight::setPower( float newVal ) {
  power = newVal;
};

float ReLight::getEfficiency() const {
  return efficiency;
};
void ReLight::setEfficiency( float newVal ) {
  efficiency = newVal;
};

bool ReLight::getInvertedNormals() const {
  return invertedNormals;
};
void ReLight::setInvertedNormals( bool newVal ) {
  invertedNormals = newVal;
};

float ReLight::getAngle() const {
  return angle;
};

void ReLight::setAngle( float newVal ) {
  angle = newVal;
};

float ReLight::getFeather() const {
  return feather;
};

void ReLight::setFeather( float newVal ) {
  feather = newVal;
};

float ReLight::getSunTurbidity() const {
  return sunTurbidity;
};
void ReLight::setSunTurbidity( float newVal ) {
  sunTurbidity = newVal;
};

float ReLight::getSkyTurbidity() const {
  return skyTurbidity;
};
void ReLight::setSkyTurbidity( float newVal ) {
  skyTurbidity = newVal;
};

float ReLight::getSkyIntensity() const {
  return skyIntensity;
};

void ReLight::setSkyIntensity( float newVal ) {
  skyIntensity = newVal;
};


float ReLight::getSunScale() const {
  return extraData.sunScale;
};

void ReLight::setSunScale( float newVal ) {
  extraData.sunScale = newVal;
};

float ReLight::getGamma() const {
  return extraData.gamma;
};

void ReLight::setGamma( float newVal ) {
  extraData.gamma = newVal;
};


float ReLight::hasNewStyleSky() const {
  return newStyleSky;
};

void ReLight::setNewStyleSky( bool newVal ) {
  newStyleSky = newVal;
};

void ReLight::setAlphaChannel( bool newVal ) {
  alphaChannel = newVal;
};

bool ReLight::hasAlphaChannel() {
  return alphaChannel;
};


QString ReLight::getTexture() {
  return imageFileName;
};

void ReLight::setTexture( const QString& newVal ) {
  imageFileName = newVal;
};

const QString ReLight::getIesFileName() const {
  return iesFileName;
};

void ReLight::setIesFileName( QString newVal ) {
  iesFileName = newVal;
};

void ReLight::setMatrix( const QVariantList& tm ) {
  matrix.setValues(tm);
}

void ReLight::setMatrix( const ReMatrix& m ) {
  matrix = m;
}


ReMatrix* ReLight::getMatrix() {
  return &matrix;
}


void ReLight::serialize( QDataStream& dataStream ) {
  dataStream << (quint16) type         << id 
             << name                   << isOn                 
             << intensity              << lightGroup
             << color
             << (quint16) colorModel   << colorTemperature    
             << presetCode             << presetDesc
             << invertedNormals        << efficiency           
             << power                  << angle 
             << feather                << extraData.sunScale
             << sunTurbidity           << skyIntensity
             << skyTurbidity           << newStyleSky
             << iesFileName            << IBLMapFileName       
             << (quint16) IBLMapFormat << IBLRotationAngle
             << alphaChannel           << fromMaterial
             << imageFileName          << isHostAreaLightFlag;

  matrix.serialize(dataStream);
}

void ReLight::deserialize( QDataStream& dataStream ) {
  quint16 typeInt, mapFormatInt;
  dataStream >> typeInt    >> id 
             >> name       >> isOn                 
             >> intensity  >> lightGroup
             >> color;
  type = static_cast<ReLightType>(typeInt);
  quint16 colorModelInt;
  dataStream >> colorModelInt    >> colorTemperature 
             >> presetCode       >> presetDesc
             >> invertedNormals  >> efficiency      
             >> power            >> angle
             >> feather          >> extraData.sunScale
             >> sunTurbidity     >> skyIntensity
             >> skyTurbidity     >> newStyleSky
             >> iesFileName      >> IBLMapFileName   
             >> mapFormatInt     >> IBLRotationAngle
             >> alphaChannel     >> fromMaterial
             >> imageFileName    >> isHostAreaLightFlag;

  dataStream >> matrix.m[0][0] >> matrix.m[0][1] >> matrix.m[0][2] >> matrix.m[0][3]
             >> matrix.m[1][0] >> matrix.m[1][1] >> matrix.m[1][2] >> matrix.m[1][3]
             >> matrix.m[2][0] >> matrix.m[2][1] >> matrix.m[2][2] >> matrix.m[2][3]
             >> matrix.m[3][0] >> matrix.m[3][1] >> matrix.m[3][2] >> matrix.m[3][3];

  colorModel = static_cast<LightColorModel>(colorModelInt);
  IBLMapFormat  = static_cast<IBLType>(mapFormatInt);
}


QString ReLight::toString() {
  return QString("ReLight: %1 -- %2, type: %3, color model: %4, intensity: %5, color: %6 angle: %7, feather: %8, is on: %9, texture: %10")
           .arg(id)
           .arg(name)
           .arg(getGetTypeAsString())
           .arg(colorModel)
           .arg(intensity)
           .arg(color.name())
           .arg(angle)
           .arg(feather)
           .arg(isOn ? "yes" : "no")
           .arg(imageFileName);
}

/*
 * IBL
 */

ReLight::IBLType ReLight::getIBLMapFormat() const {
  return IBLMapFormat;
};

void ReLight::setIBLMapFormat( IBLType newVal ) {
  IBLMapFormat = newVal;
};

void ReLight::setIBLMapFormat( const QString& newVal ) {
  QString mapFormat = newVal.toLower();
  if (mapFormat == "angular") {
    IBLMapFormat = Angular;
  }
  else if (mapFormat == "latlong") {
    IBLMapFormat = LatLong;
  }
};


float ReLight::getIBLRotationAngle() const {
  return IBLRotationAngle;
};

void ReLight::setIBLRotationAngle( const float newVal ) {
  // Make sure that the value is in the +-359 degrees value
  int times = newVal / 360;
  IBLRotationAngle = newVal - (360.f * times);
};

const QString ReLight::getIBLMapFileName() const {
  return IBLMapFileName;
};

void ReLight::setIBLMapFileName( QString newVal ) {
  // Check if the file name is the configuration file for a sIBL set
  if (!newVal.endsWith(".ibl")) {
    IBLMapFileName = newVal;
    // It's a simple file map, we're done
    return;
  }
  QFile sIBLFileRef(newVal);
  if ( !sIBLFileRef.exists() ) {
    return;
  }
  if ( sIBLFileRef.open(QIODevice::ReadOnly) ) {
    QString iblContent = sIBLFileRef.readAll();
    sIBLFileRef.close();
    // The string to search is formatted: BGFile = "filename"
    QRegExp rePreviewMap("BGfile\\s+=\\s+\"([^\"]+)\"");
    // The string to search is formatted: REFfile = "filename"
    QRegExp reIBLMap("REFfile\\s+=\\s+\"([^\"]+)\"");
    // The string to search is formatted: EVgamma = float
    QRegExp reGamma("EVgamma\\s+=\\s+([\\d\\.]+)");
    QString previewMap, IBLMap;
    float IBLGamma = 1.0;
    QFileInfo sIBLInfo(newVal);
    if (rePreviewMap.indexIn(iblContent) != -1) {
      previewMap = QString("%1/%2")
                     .arg(sIBLInfo.absolutePath())
                     .arg(rePreviewMap.cap(1));
    }
    if (reIBLMap.indexIn(iblContent) != -1) {
      IBLMap = QString("%1/%2")
                 .arg(sIBLInfo.absolutePath())
                 .arg(reIBLMap.cap(1));
    }
    if (reGamma.indexIn(iblContent) != -1) {
      IBLGamma = reGamma.cap(1).toFloat();
    }
    if (!IBLMap.isEmpty()) {
      IBLMapFileName = IBLMap;
      setTexture(previewMap);
      setGamma(IBLGamma);
      turnOnOff(true);
      //! Let the host-app know that the IBL preview should be updated
      if (!RealitySceneData->isInGUIMode()) {
        // Set the IBL preview in the host app
        RealityBase* rb = RealityBase::getRealityBase();
        rb->commandStackPush(previewMap.toUtf8());
        // sip => Set IBL Preview
        rb->commandStackPush("sip");
      }
    }
  }
};

// A Paolo Ciccone production.
