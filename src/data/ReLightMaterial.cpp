/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLightMaterial.h"

#include "ReGeometryObject.h"
#include "ReTools.h"


namespace Reality {

ReLightMaterial::ReLightMaterial( const QString matName, 
                                  const ReGeometryObject* parent ) : 
  ReMaterial(matName, parent),
  originalType(MatUndefined)
{  
  type = originalType = MatLight;
  QString lightLabel;
  // Create a unique ID so that multiple materials from the
  // same object can be turned to lights without conflicts
  lightID = QString("%1::%2").arg(parent->getInternalName()).arg(matName);
  lightLabel = QString("%1_%2").arg(parent->getName()).arg(matName);

  light = ReLightPtr( new ReLight(lightLabel, lightID, MeshLight) );  
  light->setFromMaterial(true);
}

ReLightMaterial::~ReLightMaterial() {
  light.clear();
};

void ReLightMaterial::fromMaterial( const ReMaterial* baseMat ) {
  ReTexturePtr diffuse;
  if ( !(diffuse = baseMat->getChannel("Kd")).isNull() ) {
    QString imageMap = searchImageMap(diffuse);
    light->setTexture(imageMap);
  }
}

ReLightPtr ReLightMaterial::getLight() {
  return light;
}

void ReLightMaterial::serialize( QDataStream& dataStream ) const {
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << lightID << static_cast<quint16>(originalType);
  light->serialize(dataStream);
};

void ReLightMaterial::deserialize( QDataStream& dataStream ) {
  ReMaterial::deserialize( dataStream );
  quint16 oTypeInt;
  dataStream >> lightID >> oTypeInt;
  
  originalType = static_cast<ReMaterialType>(oTypeInt);
  light = ReLightPtr( new ReLight("tmp", lightID) );
  light->deserialize(dataStream);
};



} // namespace
