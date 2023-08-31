/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReAlphaChannelMaterial.h"

#include "ReAcsel.h"
#include "ReGeometryObject.h"
#include "ReTextureCreator.h"
#include "ReTools.h"


namespace Reality {

#define RE_ALPHA_CHANNEL_NAME "alpha"

ReAlphaChannelMaterial::ReAlphaChannelMaterial( const QString name, const ReGeometryObject* parent ) :
  ReMaterial(name,parent),
  alphaStrength(1.0) 
{
  channels[RE_ALPHA_CHANNEL_NAME] = ReTexturePtr();
}

void ReAlphaChannelMaterial::fromMaterial( const ReMaterial* srcMat ) {
  auto tex = srcMat->getChannel(RE_ALPHA_CHANNEL_NAME);
  if (!tex.isNull()) {
    channels[RE_ALPHA_CHANNEL_NAME] = ReTexturePtr(
                                     ReTextureCreator::createTexture(tex->getName(),
                                                                   tex)
                                     );    
    channels[RE_ALPHA_CHANNEL_NAME]->reparent(this);
    addTextureToCatalog(channels[RE_ALPHA_CHANNEL_NAME]);    
  }
  // Test if this material has an alpha channel
  auto testMat = dynamic_cast<const ReAlphaChannelMaterial*>(srcMat);
  if (testMat) {
    setAlphaStrength(srcMat->getNamedValue("alphaStrength").toFloat());
  }
  else {
    setAlphaStrength(1.0);
  }
}

void ReAlphaChannelMaterial::setAlphaMap( ReTexturePtr& m ) {
  // alphaMap = m;
  if (!m.isNull()) {
    m->reparent(this);
    addTextureToCatalog(m);
    channels[RE_ALPHA_CHANNEL_NAME] = m;
  }
}

ReTexturePtr ReAlphaChannelMaterial::getAlphaMap() const {
  return channels[RE_ALPHA_CHANNEL_NAME];
}

/*
 Method: getAlphaStrength
 */
float ReAlphaChannelMaterial::getAlphaStrength() const {
  return alphaStrength;
};

/*
 Method: setAlphaStrength
 */
void ReAlphaChannelMaterial::setAlphaStrength( float newVal ) {
  alphaStrength = newVal;
};

void ReAlphaChannelMaterial::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "alphaStrength") {
    alphaStrength = value.toFloat();
  }
  else if (vname == "alphaMap") {
    channels[RE_ALPHA_CHANNEL_NAME] = value.value<ReTexturePtr>();
  }  
  else {
    ReMaterial::setNamedValue(vname, value);
  }
}

const QVariant ReAlphaChannelMaterial::getNamedValue( const QString& vname) const {
  QVariant val;
  if (vname == "alphaStrength") {
    return alphaStrength;
  }
  else if (vname == "alphaMap") {
    val.setValue(channels[RE_ALPHA_CHANNEL_NAME]);
  }
  else {
    return ReMaterial::getNamedValue(vname);
  }
  return val;
}

void ReAlphaChannelMaterial::serialize( QDataStream& dataStream ) const {
  dataStream << alphaStrength;
}


void ReAlphaChannelMaterial::deserialize( QDataStream& dataStream ) {
  dataStream >> alphaStrength;
}

QString ReAlphaChannelMaterial::computeAcselID( const bool forDefaultShader ) {
  QStringList acselTextures;
  auto acsel = ReAcsel::getInstance();
  if (!forDefaultShader) {
    QVariantMap origShaderData;

    if ( acsel->getOriginalShader(parent->getInternalName(), name, origShaderData) ) {
      getImageMapsOrColors(origShaderData, acselTextures);
    }    
  }
  return acsel->getAcselID(parent->getGeometryFileName(), name, acselTextures);
}


} // namespace

