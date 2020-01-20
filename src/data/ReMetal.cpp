/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReMetal.h"
#include "ReDisplaceableMaterial.h"
#include "ReTools.h"
#include "ReTextureCreator.h"

namespace Reality {


ReMetal::ReMetal( const QString name, const ReGeometryObject* parent ) : 
  DisplaceableMaterial(name, parent),
  metalType(Aluminum),
  hPolish(RE_METAL_DEFAULT_POLISH),
  vPolish(RE_METAL_DEFAULT_POLISH)
{  
  type = originalType = MatMetal;
  channels["Kr"] = ReTexturePtr();
}


void ReMetal::fromMaterial( const ReMaterial* baseMat ) {
  DisplaceableMaterial::fromMaterial(baseMat);
  auto tex = baseMat->getChannel("Kd");
  if (!tex.isNull()) {
    channels["Kr"] = ReTexturePtr(
                       TextureCreator::createTexture(tex->getName(), tex)
                     );
    channels["Kr"]->reparent(this);
    addTextureToCatalog(channels["Kr"]);
    if (!findImageMapTexture(tex).isNull() || !(isPureWhite(tex) || isPureBlack(tex))) {
      setMetalType(Custom);
    } 
  }
}
/*
 Method: setKr
 */
void ReMetal::setKr( ReTexturePtr newVal ) {
  channels["Kr"] = newVal;
  addTextureToCatalog(channels["Kr"]);
  channels["Kr"]->reparent(this);
};

ReMetal::MetalType ReMetal::getMetalType() const {
  return metalType;
};

void ReMetal::setMetalType( MetalType newVal ) {
  metalType = newVal;
};

float ReMetal::getHPolish() const {
  return hPolish;
};
void ReMetal::setHPolish( float newVal ) {
  hPolish = newVal;
};

float ReMetal::getVPolish() const {
  return vPolish;
};
void ReMetal::setVPolish( float newVal ) {
  vPolish = newVal;
};

void ReMetal::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "metalType") {
    metalType = static_cast<MetalType>(value.toInt());
  }
  else if (name == "hPolish") {
    hPolish = value.toInt();
  }
  else if (name == "vPolish") {
    vPolish = value.toInt();
  }
  else if (name == "Kr") {
    channels["Kr"] = value.value<ReTexturePtr>();
  }
  else {
    DisplaceableMaterial::setNamedValue(name, value);
  }
}

const QVariant ReMetal::getNamedValue( const QString& name ) const {
  QVariant val;
  if (name == "metalType") {
    return metalType;
  }
  else if (name == "hPolish") {
    return hPolish;
  }
  else if (name == "vPolish") {
    return vPolish;
  }
  else if (name == "Kr") {
    val.setValue(channels["Kr"]);
  }
  else {
    val = DisplaceableMaterial::getNamedValue(name);
  }
  return val;
}

void ReMetal::serialize( QDataStream& dataStream ) const { 
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << (quint16) metalType << hPolish << vPolish;
  DisplaceableMaterial::serialize(dataStream);      
};


void ReMetal::deserialize( QDataStream& dataStream ) {
  ReMaterial::deserialize( dataStream );
  quint16 intMetalType;
  dataStream >> intMetalType >> hPolish >> vPolish;
  metalType = static_cast<MetalType>(intMetalType);
  DisplaceableMaterial::deserialize(dataStream);      
}

} // namespace
