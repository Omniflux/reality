/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReGlossy.h"

#include "ReTextureCreator.h"
#include "textures/ReConstant.h"


//! Medium gray
const QColor RE_GLOSSY_DEFAULT_KS(25, 25, 25);

namespace Reality {

ReGlossy::ReGlossy( const QString name, const ReGeometryObject* parent) :
  DisplaceableMaterial(name, parent) {
  type          = originalType = MatGlossy;
  uGlossiness   = vGlossiness = 5000;
  surfaceFuzz   = false;
  topCoat       = false;
  translucent   = false;
  coatThickness = RE_GLOSSY_COAT_DEFAULT_THICKNESS;

  channels[RE_GLOSSY_KD_CH] = ReTexturePtr(
                                new ReConstant(QString("%1_Kd").arg(name), 
                                               this,
                                               RE_WHITE_COLOR)
                              );
  // channels[RE_GLOSSY_KD_CH].staticCast<ReConstant>()->setColor(RE_WHITE_COLOR);

  channels[RE_GLOSSY_KS_CH] = ReTexturePtr(
                                new ReConstant(QString("%1_Ks").arg(name), 
                                               this,
                                               RE_GLOSSY_DEFAULT_KS));
  // channels[RE_GLOSSY_KS_CH].staticCast<ReConstant>()->setColor(RE_GLOSSY_DEFAULT_KS);

  channels[RE_GLOSSY_KA_CH] = ReTexturePtr(new Reality::ReConstant(QString("%1_Ka").arg(name), this));
  //  For Glossy Translucent
  channels[RE_GLOSSY_KT_CH] = ReTexturePtr(new Reality::ReConstant(QString("%1_Kt").arg(name), this));
  // The glossiness driver
  channels[RE_GLOSSY_KG_CH] = ReTexturePtr();

  channels[RE_GLOSSY_KD_CH]->reparent(this);
  channels[RE_GLOSSY_KS_CH]->reparent(this);
  channels[RE_GLOSSY_KA_CH]->reparent(this);
  channels[RE_GLOSSY_KT_CH]->reparent(this);
  addTextureToCatalog(channels[RE_GLOSSY_KD_CH]);
  addTextureToCatalog(channels[RE_GLOSSY_KS_CH]);
  addTextureToCatalog(channels[RE_GLOSSY_KA_CH]);
  addTextureToCatalog(channels[RE_GLOSSY_KT_CH]);
}

void ReGlossy::fromMaterial( const ReMaterial* baseMat ) {
  DisplaceableMaterial::fromMaterial(baseMat);
  auto tex = baseMat->getChannel(RE_GLOSSY_KD_CH);
  if (!tex.isNull()) {
    channels[RE_GLOSSY_KD_CH] = ReTexturePtr(ReTextureCreator::createTexture(tex->getName(), tex));
  }
  else {
    switch( baseMat->getType() ) {
      case MatMirror:
      case MatMetal : {
        tex = baseMat->getChannel("Kr");
        channels[RE_GLOSSY_KD_CH] = ReTexturePtr(
                           ReTextureCreator::createTexture(tex->getName(), tex)
                         );
        break;
      }

      case MatGlass: {
        tex = baseMat->getChannel(RE_GLOSSY_KT_CH);
        channels[RE_GLOSSY_KD_CH] = ReTexturePtr(
                           ReTextureCreator::createTexture(tex->getName(), tex)
                         );
        break;
      }
      default:
        break;
    }    
  }
  channels[RE_GLOSSY_KD_CH]->reparent(this);
  addTextureToCatalog(channels[RE_GLOSSY_KD_CH]);
  
  tex = baseMat->getChannel(RE_GLOSSY_KS_CH);
  if (!tex.isNull()) {
    channels[RE_GLOSSY_KS_CH] = ReTexturePtr(ReTextureCreator::createTexture(tex->getName(), tex));
    channels[RE_GLOSSY_KS_CH]->reparent(this);
    addTextureToCatalog(channels[RE_GLOSSY_KS_CH]);
  }
  tex = baseMat->getChannel(RE_GLOSSY_KA_CH);
  if (!tex.isNull()) {
    channels[RE_GLOSSY_KA_CH] = ReTexturePtr(ReTextureCreator::createTexture(tex->getName(), tex));
    channels[RE_GLOSSY_KA_CH]->reparent(this);
    addTextureToCatalog(channels[RE_GLOSSY_KA_CH]);
  }
  tex = baseMat->getChannel(RE_GLOSSY_KT_CH);
  if (!tex.isNull() && channels.contains(RE_GLOSSY_KT_CH)) {
    channels[RE_GLOSSY_KT_CH] = ReTexturePtr(ReTextureCreator::createTexture(tex->getName(), tex));
    channels[RE_GLOSSY_KT_CH]->reparent(this);
    addTextureToCatalog(channels[RE_GLOSSY_KT_CH]);
    setTranslucent(baseMat->getNamedValue("translucent").toBool());
  }
  tex = baseMat->getChannel(RE_GLOSSY_KG_CH);
  if (!tex.isNull()) {
    channels[RE_GLOSSY_KG_CH] = ReTexturePtr(
                                  ReTextureCreator::createTexture(
                                    tex->getName(), tex
                                  )
                                );
    channels[RE_GLOSSY_KG_CH]->reparent(this);
    addTextureToCatalog(channels[RE_GLOSSY_KG_CH]);
  }
  if (baseMat->getType() == MatGlossy) {
    uGlossiness = baseMat->getNamedValue("uGlossiness").toFloat();
    vGlossiness = baseMat->getNamedValue("vGlossiness").toFloat();
    surfaceFuzz = baseMat->getNamedValue("surfaceFuzz").toBool();
  }
}

ReGlossy::~ReGlossy() {
}

void ReGlossy::setKd(const ReTexturePtr Kd) {
  // Clear the channel
  setChannel(RE_GLOSSY_KD_CH, "");
  channels[RE_GLOSSY_KD_CH] = Kd;
  channels[RE_GLOSSY_KD_CH]->reparent(this);
  addTextureToCatalog(channels[RE_GLOSSY_KD_CH]);
}

void ReGlossy::setKs(const ReTexturePtr newKs) {
  // Clear the channel
  setChannel(RE_GLOSSY_KS_CH, "");

  channels[RE_GLOSSY_KS_CH] = newKs;
  channels[RE_GLOSSY_KS_CH]->reparent(this);
  addTextureToCatalog(channels[RE_GLOSSY_KS_CH]);
  // // The specular map needs to have linear gamma
  // setTextureGamma(channels[RE_GLOSSY_KS_CH], 1.0);
}

void ReGlossy::setKa(const ReTexturePtr newKa) {
  // Clear the channel
  setChannel(RE_GLOSSY_KA_CH, "");

  channels[RE_GLOSSY_KA_CH] = newKa;
  channels[RE_GLOSSY_KA_CH]->reparent(this);
  addTextureToCatalog(channels[RE_GLOSSY_KA_CH]);
}

void ReGlossy::setKt(const ReTexturePtr newKt) {
  // Clear the channel
  setChannel(RE_GLOSSY_KT_CH, "");

  channels[RE_GLOSSY_KT_CH] = newKt;
  channels[RE_GLOSSY_KT_CH]->reparent(this);
  addTextureToCatalog(channels[RE_GLOSSY_KT_CH]);
}

const ReTexturePtr ReGlossy::getKg() const {
  return channels[RE_GLOSSY_KG_CH];
};

void ReGlossy::setKg( ReTexturePtr newVal ) {
  channels[RE_GLOSSY_KG_CH] = newVal;
  channels[RE_GLOSSY_KG_CH]->reparent(this);
  addTextureToCatalog(channels[RE_GLOSSY_KG_CH]);  
};

void ReGlossy::serialize( QDataStream& dataStream ) const {
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << uGlossiness << vGlossiness 
             << surfaceFuzz << coatThickness 
             << translucent << topCoat;
  DisplaceableMaterial::serialize(dataStream);
};

void ReGlossy::deserialize( QDataStream& dataStream ) {
  ReMaterial::deserialize( dataStream );
  dataStream >> uGlossiness >> vGlossiness 
             >> surfaceFuzz >> coatThickness 
             >> translucent >> topCoat;
  DisplaceableMaterial::deserialize(dataStream);
}


void ReGlossy::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "vGlossiness") {
    vGlossiness = value.toInt();
  }
  else if (name == "uGlossiness") {
    uGlossiness = value.toInt();
  }
  else if (name == "surfaceFuzz") {
    surfaceFuzz = value.toBool();
  }
  else if (name == "translucent") {
    translucent = value.toBool();
  }
  else if (name == "topCoat") {
    topCoat = value.toBool();
  }
  else if (name == "coatThickness") {
    coatThickness = value.toDouble();
  }
  else if (name == RE_GLOSSY_KD_CH) {
    setKd(value.value<ReTexturePtr>());
  }
  else if (name == RE_GLOSSY_KS_CH) {
    setKs(value.value<ReTexturePtr>());
  }
  else if (name == RE_GLOSSY_KA_CH) {
    setKa(value.value<ReTexturePtr>());
  }
  else if (name == RE_GLOSSY_KT_CH) {
    setKt(value.value<ReTexturePtr>());
  }
  else if (name == RE_GLOSSY_KG_CH) {
    setKg(value.value<ReTexturePtr>());
  }
  else {
    DisplaceableMaterial::setNamedValue(name, value);
  }
}


const QVariant ReGlossy::getNamedValue( const QString& name ) const {
  QVariant val;
  if (name == "vGlossiness") {
    return vGlossiness;
  }
  else if (name == "uGlossiness") {
    return uGlossiness;
  }
  else if (name == "surfaceFuzz") {
    return surfaceFuzz;
  }
  else if (name == "topCoat") {
    return topCoat;
  }
  else if (name == "translucent") {
    return translucent;
  }
  else if (name == "coatThickness") {
    return coatThickness;
  }
  else if (name == RE_GLOSSY_KD_CH) {
    val.setValue(channels[RE_GLOSSY_KD_CH]);
  }
  else if (name == RE_GLOSSY_KS_CH) {
    val.setValue(channels[RE_GLOSSY_KS_CH]);
  }
  else if (name == RE_GLOSSY_KA_CH) {
    val.setValue(channels[RE_GLOSSY_KA_CH]);
  }
  else if (name == RE_GLOSSY_KT_CH) {
    val.setValue(channels[RE_GLOSSY_KT_CH]);
  }
  else if (name == RE_GLOSSY_KG_CH) {
    val.setValue(channels[RE_GLOSSY_KG_CH]);
  }
  else {
    val = DisplaceableMaterial::getNamedValue(name);
  }
  return val;
}

QString ReGlossy::toString() {
  QString desc =  QString("Glossy: %1, %2, %3, %4, %5\n")
                    .arg(name)
                    .arg(uGlossiness)
                    .arg(vGlossiness)
                    .arg(isTranslucent() ? "Translucent" : "No translucency")
                    .arg(hasTopCoat() ? "Has coat" : "No coat");
  ReNodeDictionaryIterator i(nodeCatalog);
  while( i.hasNext() ) {
    i.next();
    desc += QString("  %1\n").arg(i.value()->toString());
  }
  ReNodeDictionaryIterator ci(channels);
  while( ci.hasNext() ) {
    ci.next();
    if (!ci.value()) {
      continue;
    }
    desc += QString("  ch: %1 => %2\n").arg(ci.key()).arg(ci.value()->getName());
  }

  return desc;
}

} // namespace

