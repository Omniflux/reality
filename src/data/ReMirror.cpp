/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReMirror.h"
#include "ReDisplaceableMaterial.h"
#include "ReTools.h"
#include "ReTextureCreator.h"

  
namespace Reality {


ReMirror::ReMirror( const QString name, const ReGeometryObject* parent ) : 
  DisplaceableMaterial(name, parent),
  filmIOR(1.5),
  filmThickness(2.0)
{
  type = originalType = MatMirror;
  channels["Kr"] = ReTexturePtr( 
    new ReConstant(QString("%1_Kr").arg(name), this, QColor(250,250,250)) 
  );
  addTextureToCatalog(channels["Kr"]);
}

void ReMirror::fromMaterial( const ReMaterial* baseMat ) {
  DisplaceableMaterial::fromMaterial(baseMat);  
  auto tex = baseMat->getChannel("Kr");
  if (!tex.isNull()) {
    channels["Kr"] = ReTexturePtr(TextureCreator::createTexture(tex->getName(), tex));
    channels["Kr"]->reparent(this);
    addTextureToCatalog(channels["Kr"]);
  }
  else {
    tex = baseMat->getChannel("Kd");
    if (!tex.isNull()) {
      channels["Kr"] = ReTexturePtr(TextureCreator::createTexture(tex->getName(), tex));
      channels["Kr"]->reparent(this);
      addTextureToCatalog(channels["Kr"]);
    }
  }
  filmIOR = 1.5;
  filmThickness = 2.0;
  alphaStrength = 1.0;
}

void ReMirror::setKr( ReTexturePtr newVal ) {
  channels["Kr"] = newVal;
  addTextureToCatalog(channels["Kr"]);
  channels["Kr"]->reparent(this);
};

float ReMirror::getFilmIOR() const {
  return filmIOR;
};
void ReMirror::setFilmIOR( float newVal ) {
  filmIOR = newVal;
};

float ReMirror::getFilmThickness() const {
  return filmThickness;
};

void ReMirror::setFilmThickness( float newVal ) {
  filmThickness = newVal;
};

void ReMirror::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "filmThickness") {
    filmThickness = value.toFloat();
  }
  else if (vname == "filmIOR") {
    filmIOR = value.toFloat();
  }
  else if (vname == "Kr") {
    channels["Kr"] = value.value<ReTexturePtr>();
    addTextureToCatalog(channels["Kr"]);
  }
  else {
    DisplaceableMaterial::setNamedValue(vname, value);
  }
}

const QVariant ReMirror::getNamedValue( const QString& vname ) const {
  if (vname == "filmThickness") {
    return filmThickness;
  }
  else if (vname == "filmIOR") {
    return filmIOR;
  }
  else if (vname == "Kr") {
    QVariant val;    
    val.setValue(channels["Kr"]);
    return val;
  }
  return DisplaceableMaterial::getNamedValue(vname);
}

void ReMirror::serialize( QDataStream& dataStream ) const {
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << filmIOR << filmThickness;
  DisplaceableMaterial::serialize(dataStream);  
}

void ReMirror::deserialize( QDataStream& dataStream ){
  ReMaterial::deserialize( dataStream );
  dataStream >> filmIOR >> filmThickness;
  DisplaceableMaterial::deserialize(dataStream);
}



} // namespace
