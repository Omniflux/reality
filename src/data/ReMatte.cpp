/**
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 * \file
 */

#include "ReMatte.h"
#include "ReTools.h"
#include "ReTextureCreator.h"

namespace Reality {

ReMatte::ReMatte(const QString name, const ReGeometryObject* parent ) : 
  DisplaceableMaterial(name, parent) {
  type = originalType = MatMatte;
  setKd(ReTexturePtr(new ReConstant(QString("%1_Kd").arg(name), this, RE_WHITE_COLOR)));
  // Translucency channel
  setKt(ReTexturePtr(new ReConstant(QString("%1_Kt").arg(name), this, RE_WHITE_COLOR)));

  roughness = 0.4;
  translucent = false;
}

void ReMatte::fromMaterial( const ReMaterial* baseMat ) {
  DisplaceableMaterial::fromMaterial(baseMat);
  auto tex = baseMat->getChannel("Kd");
  ReMaterialType otherType = baseMat->getType();
  if (!tex.isNull()) {
    channels["Kd"] = ReTexturePtr(TextureCreator::createTexture(tex->getName(), tex));
  }
  else {
    switch( otherType ) {
      case MatMirror:
      case MatMetal : {
        tex = baseMat->getChannel("Kr");
        channels["Kd"] = ReTexturePtr(
                           TextureCreator::createTexture(tex->getName(), tex)
                         );
        break;
      }

      case MatGlass: {
        tex = baseMat->getChannel("Kt");
        channels["Kd"] = ReTexturePtr(
                           TextureCreator::createTexture(tex->getName(), tex)
                         );
        break;
      }
      default:
        break;
    }
  }
  channels["Kd"]->reparent(this);
  addTextureToCatalog(channels["Kd"]);

  tex = baseMat->getChannel("Kt");
  if (!tex.isNull()) {
    channels["Kt"] = ReTexturePtr(TextureCreator::createTexture(tex->getName(), tex));
    channels["Kt"]->reparent(this);
    addTextureToCatalog(channels["Kt"]);
    if (otherType == MatGlossy) {
      translucent = baseMat->getNamedValue("translucent").toBool();
    }
  }
}

void ReMatte::setKd(const ReTexturePtr _Kd) {
  channels["Kd"] = _Kd;
  addTextureToCatalog(channels["Kd"]);  
  channels["Kd"]->reparent(this);
}

void ReMatte::setKt(const ReTexturePtr _Kt) {
  channels["Kt"] = _Kt;
  addTextureToCatalog(channels["Kt"]);  
  channels["Kt"]->reparent(this);
}

void ReMatte::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "roughness") {
    roughness = value.toFloat();
  }
  else if (vname == "conserveEnergy") {
    conserveEnergy = value.toBool();
  }
  else if (vname == "translucent") {
    translucent = value.toBool();
  }
  else {
    DisplaceableMaterial::setNamedValue(vname, value);
  }
}

const QVariant ReMatte::getNamedValue( const QString& vname ) const {
  if (vname == "roughness") {
    return roughness;
  }
  else if (vname == "conserveEnergy") {
    return conserveEnergy;
  }
  else if (vname == "translucent") {
    return translucent;
  }
  else {
    return DisplaceableMaterial::getNamedValue(vname);
  }
}


void ReMatte::serialize( QDataStream& dataStream ) const {
  // Call the base class serializer
  ReMaterial::serialize(dataStream);

  dataStream << roughness << conserveEnergy << translucent;
  DisplaceableMaterial::serialize(dataStream);  
}


void ReMatte::deserialize( QDataStream& dataStream ){
  ReMaterial::deserialize( dataStream );
  dataStream >> roughness >> conserveEnergy >> translucent;
  DisplaceableMaterial::deserialize(dataStream);
}


} // namespace

