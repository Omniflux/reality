/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReGlass.h"
#include "ReAcsel.h"
#include "ReTools.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReTextureCreator.h"

// Default glass IOR, Fused Silica Glass
#define  RE_GLASS_DEFAULT_IOR 1.459f

namespace Reality {

ReGlass::ReGlass( const QString name, const ReGeometryObject* parent ) : 
  DisplaceableMaterial(name, parent),
  glassType(StandardGlass),
  uRoughness(0.0),
  vRoughness(0.0),
  thinFilmIOR(0.0),
  thinFilmThickness(1.2f),
  IOR(RE_GLASS_DEFAULT_IOR),
  dispersion(false),
  cauchyB(RE_GLASS_DEFAULT_CAUCHYB) // Good value for generic glass
{
  type = originalType = MatGlass;  
  setKr( ReConstantPtr(new Reality::ReConstant("glass_Kr", this, QColor(255,255,255))) );
  setKt( ReConstantPtr(new Reality::ReConstant("glass_Kt", this, QColor(255,255,255))) );
}

void ReGlass::fromMaterial( const ReMaterial* baseMat ) {
  DisplaceableMaterial::fromMaterial(baseMat);
  alphaStrength = 1.0;
  auto tex = baseMat->getChannel("Kd");
  if (!tex.isNull()) {
    setKt(ReTexturePtr(TextureCreator::createTexture(tex->getName(), tex)));
  }
  // If we assigned to Kt then we are all done
  if (!tex.isNull()) {
    return;
  }

  tex = baseMat->getChannel("Kt");
  if (!tex.isNull()) {
    setKt(ReTexturePtr(TextureCreator::createTexture(tex->getName(), tex)));
  }
}

void ReGlass::textureUpdated( const QString& textureName ) {
  if (glassType == VolumetricGlass) {
    if (innerVolume != "") {
      ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
      ReTexturePtr tex = getTexture(textureName);
      if (!vol.isNull() && !tex.isNull() && tex->getType() == TexConstant) {
        vol->setColor(tex->getNamedValue("color").value<QColor>());
      }
    }
  }
}

void ReGlass::setKt( ReTexturePtr newVal ) {
  channels["Kt"] = newVal;
  channels["Kt"]->reparent(this);
  addTextureToCatalog(channels["Kt"]);  
};

void ReGlass::setKr( ReTexturePtr newVal ) {
  channels["Kr"] = newVal;
  channels["Kr"]->reparent(this);
  addTextureToCatalog(channels["Kr"]);  
};


float ReGlass::getThinFilmThickness() const {
  return thinFilmThickness;
};

void ReGlass::setThinFilmThickness( float newVal ) {
  thinFilmThickness = newVal;
};
    

float ReGlass::getThinFilmIOR() const {
  return thinFilmIOR;
};

void ReGlass::setThinFilmIOR( float newVal ) {
  thinFilmIOR = newVal;
};


ReGlass::GlassType ReGlass::getGlassType() const {
  return glassType;
};

void ReGlass::setGlassType( GlassType newVal ) {
  glassType = newVal;
};

void ReGlass::serialize( QDataStream& dataStream ) const { 
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << (quint16) glassType
             << uRoughness  << vRoughness 
             << thinFilmIOR << thinFilmThickness 
             << IOR         << IORLabel
             << dispersion  << cauchyB;
  DisplaceableMaterial::serialize(dataStream);      
};



void ReGlass::deserialize( QDataStream& dataStream ) {
  ReMaterial::deserialize( dataStream );

  quint16 typeInt;
  dataStream >> typeInt
             >> uRoughness  >> vRoughness 
             >> thinFilmIOR >> thinFilmThickness 
             >> IOR         >> IORLabel
             >> dispersion  >> cauchyB;
  glassType = static_cast<GlassType>(typeInt);
  
  DisplaceableMaterial::deserialize(dataStream);      
}


void ReGlass::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "glassType") {
    glassType = static_cast<GlassType>(value.toInt());
  }
  else if (name == "uRoughness") {
    uRoughness = value.toDouble();
  }
  else if (name == "vRoughness") {
    vRoughness = value.toDouble();
  }
  else if (name == "thinFilmIOR") {
    thinFilmIOR = value.toDouble();
  }
  else if (name == "thinFilmThickness") {
    thinFilmThickness = value.toDouble();
  }
  else if (name == "dispersion") {
    dispersion = value.toBool();
  }
  else if (name == "cauchyB") {
    cauchyB = value.toFloat();
  }
  else if (name == "IOR") {
    IOR = value.toDouble();
  }
  else if (name == "IORLabel") {
    IORLabel = value.toString();
  }
  else if (name == "Kt") {
    channels["Kt"] = value.value<ReTexturePtr>();
  }
  else if (name == "Kr") {
    channels["Kr"] = value.value<ReTexturePtr>();
  }
  else {
    DisplaceableMaterial::setNamedValue(name, value);
  }
}

const QVariant ReGlass::getNamedValue( const QString& name ) const {
  QVariant val;
  if (name == "glassType") {
    return glassType ;
  }
  else if (name == "uRoughness") {
    return uRoughness;
  }
  else if (name == "vRoughness") {
    return vRoughness;
  }
  else if (name == "thinFilmIOR") {
    return thinFilmIOR;
  }
  else if (name == "dispersion") {
    return dispersion;
  }
  else if (name == "cauchyB") {
    return cauchyB;
  }
  else if (name == "thinFilmThickness") {
    return thinFilmThickness;
  }
  else if (name == "IOR") {
    return IOR;
  }
  else if (name == "IORLabel") {
    return IORLabel;
  }
  else if (name == "Kt") {
    val.setValue(channels["Kt"]);
  }
  else if (name == "Kr") {
    val.setValue(channels["Kr"]);
  }
  else {
    val = DisplaceableMaterial::getNamedValue(name);
  }

  return val;
}

// OBSOLETE, replaced by correct version in ReAlphaChannelMaterial
/*
QString ReGlass::computeAcselID( const bool forDefaultShader ) {
  // The Glass ACSEL ID is based on the texture for the transmission channel,
  // since glass doesn't have a diffuse channel
  QStringList acselTextures;
  auto t = channels.value("Kt");
  if (!forDefaultShader) {
    getImageMaps(t, acselTextures);
    // If an image map is not found then check if the texture is a constant
    // color. If it is the use the CSS-style name of the color to identify 
    // the texture
    if (!acselTextures.count()) {
      if (t->getType() == TexConstant) {
        acselTextures << t.staticCast<ReConstant>()->getColor().name();
      }
    }

    getImageMaps(channels.value("alpha"), acselTextures);
  }
  return ReAcsel::getInstance()->getAcselID(
           parent->getGeometryFileName(), 
           name, 
           acselTextures
         );
}
*/

} // namespace
