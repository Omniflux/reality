/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReVelvet.h"

#include "ReTextureCreator.h"
#include "textures/ReConstant.h"


namespace Reality {


ReVelvet::ReVelvet( const QString name, const ReGeometryObject* parent ) : 
  ReModifiedMaterial(name, parent)
{
  type = originalType = MatVelvet;
  thickness = 1.0;  
  channels["Kd"] = ReTexturePtr( 
    new ReConstant(QString("%1_Kd").arg(name), this, QColor(48, 99, 201) ) 
  );
  addTextureToCatalog(channels["Kd"]);
}


void ReVelvet::fromMaterial( const ReMaterial* baseMat ) {
  ReModifiedMaterial::fromMaterial(baseMat);
  auto tex = baseMat->getChannel("Kd");
  if (!tex.isNull()) {
    channels["Kd"] = ReTexturePtr(ReTextureCreator::createTexture(tex->getName(), tex));
    channels["Kd"]->reparent(this);
    addTextureToCatalog(channels["Kd"]);
  }  
}


void ReVelvet::setKd( ReTexturePtr newVal ) {
  channels["Kd"] = newVal;
  addTextureToCatalog(channels["Kd"]);
  channels["Kd"]->reparent(this);
};

void ReVelvet::setThickness( float newVal ) {
  thickness = newVal;
};

void ReVelvet::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "thickness") {
    thickness = value.toFloat();
  }
  else if (vname == "Kd") {
    channels["Kd"] = value.value<ReTexturePtr>();
    addTextureToCatalog(channels["Kd"]);
  }
  else {
    ReModifiedMaterial::setNamedValue(vname, value);
  }
}

const QVariant ReVelvet::getNamedValue( const QString& vname ) const {
  if (vname == "thickness") {
    return thickness;
  }
  else if (vname == "Kd") {
    QVariant val;    
    val.setValue(channels["Kd"]);
    return val;
  }
  return ReModifiedMaterial::getNamedValue(vname);
}

void ReVelvet::serialize( QDataStream& dataStream ) const {
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << thickness;
  ReModifiedMaterial::serialize(dataStream);
}


void ReVelvet::deserialize( QDataStream& dataStream ){
  ReMaterial::deserialize( dataStream );
  dataStream >> thickness;
  ReModifiedMaterial::deserialize(dataStream);
}


} // namespace
