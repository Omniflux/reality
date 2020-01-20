/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReCamera.h"

using namespace Reality;

double ReCamera::getPoserFOV( const uint frameWidth, 
                              const uint frameHeight ) const 
{
  // Empirically-found coefficient for Poser
  const float coeff = 12.74f; 
  // 360 / PI
  const double diameterAngle = 114.591559026; 
  return(diameterAngle * 
    atan(coeff * static_cast<float>(frameHeight) / 
                   static_cast<float>(frameWidth) / focalLength));
};

double ReCamera::getFOV( const uint frameWidth, 
                               const uint frameHeight) const 
{
  HostAppID hostApp = RealityBase::getRealityBase()->getHostAppID();

  switch (hostApp) {
    case DAZStudio: {
      return getFOV();
    }
    case Poser: {
      return getPoserFOV(frameWidth, frameHeight);      
    }    
  }
  return 0.0;
};


QString& ReCamera::toString() {
  toStringValue = QString("camera: %1, %2, %3, %4, %5, %6")
                    .arg(name)
                    .arg(focalLength)
                    .arg(focalDistance)
                    .arg(hither)
                    .arg(yon)
                    .arg(getMatrix()->toString());
  return toStringValue;
}

void ReCamera::setNamedValue( const QString& valueName, const QVariant& value) {
  if (valueName == "name") {
    name = value.toString();
  }
  else if (valueName == "ID") {
    ID = value.toString();
  }
  else if (valueName == "focalLength") {
    focalLength = value.toFloat();
  }
  else if (valueName == "focalDistance") {
    focalDistance = value.toFloat();
  }
  else if (valueName == "fStop") {
    fStop = value.toFloat();
  }
  else if (valueName == "dofEnabled") {
    dofEnabled = value.toBool();
  }
  else if (valueName == "exposureEnabled") {
    exposureEnabled = value.toBool();
  }
  else if (valueName == "iso") {
    iso = value.toInt();
  }
  else if (valueName == "shutter") {
    shutter = value.toFloat();
  }
  else if (valueName == "dofStop") {
    dofStop = value.toFloat();
  }
  else if (valueName == "filmResponse") {
    filmResponse = value.toString();
  }
}

void ReCamera::serialize( QDataStream& dataStream ) {
  dataStream << name        << ID              << scale
             << focalLength << focalDistance   << fStop
             << dofEnabled  << exposureEnabled << iso
             << shutter     << dofStop         << filmResponse
             << fov;
  matrix.serialize(dataStream);
}

void ReCamera::deserialize( QDataStream& dataStream ) {
  dataStream >> name        >> ID              >> scale
             >> focalLength >> focalDistance   >> fStop
             >> dofEnabled  >> exposureEnabled >> iso
             >> shutter     >> dofStop         >> filmResponse
             >> fov;
  matrix.deserialize(dataStream);
}

