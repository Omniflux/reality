/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReDisplaceableMaterial.h"

#include "ReTextureCreator.h"


namespace Reality {

DisplaceableMaterial::DisplaceableMaterial( const QString name, const ReGeometryObject* parent ) :
  ReAlphaChannelMaterial(name,parent),
  isNormalMap(false),
  bmNegative(-0.001f),
  bmPositive(0.001f),
  bmStrength(0.5f),
  dmNegative(-0.01f),
  dmPositive(0.01f),
  dmStrength(1.0f),
  subdivision(0),
  useMicrofacets(false),
  smoothnessFlag(true),
  keepSharpEdgesFlag(true),
  emitsLight(false),
  lightGain(1.0)
{
  // Bump channel
  channels["Bm"] = ReTexturePtr();
  // Distortion channel
  channels["Dm"] = ReTexturePtr();
  // Ambient channel
  channels["Kl"] = ReTexturePtr();  
}

ReTexturePtr DisplaceableMaterial::getBumpMap() const {
  return channels["Bm"];
};

void DisplaceableMaterial::setBumpMap( ReTexturePtr& newVal ) {
  channels["Bm"] = newVal;
  channels["Bm"]->reparent(this);
  addTextureToCatalog(channels["Bm"]);
};

bool DisplaceableMaterial::hasBumpMap() const {
  return !channels["Bm"].isNull();
}


float DisplaceableMaterial::getBmPositive() const {
  return bmPositive;
};

void DisplaceableMaterial::setBmPositive( float newVal ) {
  bmPositive = newVal;
};


float DisplaceableMaterial::getBmNegative() const {
  return bmNegative;
};

void DisplaceableMaterial::setBmNegative( float newVal ) {
  bmNegative = newVal;
};


float DisplaceableMaterial::getBmStrength() const {
  return bmStrength;
};

void DisplaceableMaterial::setBmStrength( float newVal ) {
  bmStrength = newVal;
};
  
bool DisplaceableMaterial::hasNormalMap() const {
  return isNormalMap;
}

void DisplaceableMaterial::setNormalMapFlag( const bool f ) {
  isNormalMap = f;
};

void DisplaceableMaterial::setDisplacementMap( ReTexturePtr newVal ) {
  channels["Dm"] = newVal;
  channels["Dm"]->reparent(this);
  addTextureToCatalog(channels["Dm"]);  
};

bool DisplaceableMaterial::hasDisplacementMap() const {
  return !channels["Dm"].isNull();
}

void DisplaceableMaterial::setDmPositive( float newVal ) {
  dmPositive = newVal;
};


void DisplaceableMaterial::setDmNegative( float newVal ) {
  dmNegative = newVal;
};

void DisplaceableMaterial::setDmStrength( float newVal ) {
  dmStrength = newVal;
};

ReTexturePtr DisplaceableMaterial::getAmbientMap() const {
  return channels["Kl"];
};

void DisplaceableMaterial::setAmbientMap( ReTexturePtr& newVal ) {
  channels["Kl"] = newVal;
  channels["Kl"]->reparent(this);
  addTextureToCatalog(channels["Kl"]);
};

void DisplaceableMaterial::setSubdivisions( quint16 newVal ) {
  subdivision = newVal;
};

void DisplaceableMaterial::setUseMicrofacets( bool newVal ) {
  useMicrofacets = newVal;
};

void DisplaceableMaterial::setIsSmooth( bool newVal ) {
  smoothnessFlag = newVal;
};

void DisplaceableMaterial::setKeepSharpEdges( bool newVal ) {
  keepSharpEdgesFlag = newVal;
};

void DisplaceableMaterial::setEmitsLight( const bool newVal ) {
  emitsLight = newVal;
};

void DisplaceableMaterial::setLightGain( const float newVal ) {
  lightGain = newVal;
};

const QVariant DisplaceableMaterial::getNamedValue( const QString& name) const {
  QVariant val;
  if (name == "Bm" || name == "bumpMap") {
    val.setValue(channels["Bm"]);
  }
  else if (name == "isNormalMap") {
    return isNormalMap;
  }
  else if (name == "bmNegative") {
    return bmNegative;
  }
  else if (name == "bmPositive") {
    return bmPositive;
  }
  else if (name == "bmStrength") {
    return bmStrength;
  }
  else if (name == "Dm" || name == "displacementMap") {
    val.setValue(channels["Dm"]);
  }
  else if (name == "dmNegative") {
    return dmNegative;
  }
  else if (name == "dmPositive") {
    return dmPositive;
  }
  else if (name == "dmStrength") {
    return dmStrength;
  }
  else if (name == "subdivision") {
    return subdivision;
  }
  else if (name == "useMicrofacets") {
    return useMicrofacets;
  }
  else if (name == "smoothnessFlag") {
    return smoothnessFlag;
  }
  else if (name == "keepSharpEdgesFlag") {
    return keepSharpEdgesFlag;
  }
  else if (name == "emitsLight") {
    return emitsLight;
  }
  else if (name == "lightGain") {
    return lightGain;
  }
  else if (name == "Kl" || name == "ambientMap") {
    val.setValue(channels["Kl"]);
  }
  else {
    val = ReAlphaChannelMaterial::getNamedValue(name);
  }
  return val;
}

void DisplaceableMaterial::setNamedValue( const QString& name, const QVariant& value ) {
  if (name == "Bm" || name == "bumpMap") {
    channels["Bm"] = value.value<ReTexturePtr>();
  }
  else if (name == "isNormalMap") {
    isNormalMap = value.toBool();
  }
  else if (name == "bmNegative") {
    bmNegative = value.toFloat();
  }
  else if (name == "bmPositive") {
    bmPositive = value.toFloat();
  }
  else if (name == "bmStrength") {
    bmStrength = value.toFloat();
  }
  else if (name == "Dm" || name == "displacementMap") {
    channels["Dm"] = value.value<ReTexturePtr>();
  }
  else if (name == "dmNegative") {
    dmNegative = value.toFloat();
  }
  else if (name == "dmPositive") {
    dmPositive = value.toFloat();
  }
  else if (name == "dmStrength") {
    dmStrength = value.toFloat();
  }
  else if (name == "subdivision") {
    subdivision = value.toInt();
  }
  else if (name == "useMicrofacetsFlag") {
    useMicrofacets = value.toBool();
  }
  else if (name == "smoothnessFlag") {
    smoothnessFlag = value.toBool();
  }
  else if (name == "keepSharpEdges") {
    keepSharpEdgesFlag = value.toBool();
  }
  else if (name == "emitsLight") {
    emitsLight = value.toBool();
  }
  else if (name == "lightGain") {
    lightGain = value.toFloat();
  }
  else if (name == "Kl" || name == "ambientMap") {
    channels["Kl"] = value.value<ReTexturePtr>();
  }
  else {
    ReAlphaChannelMaterial::setNamedValue(name, value);
  }
}

void DisplaceableMaterial::fromMaterial( const ReMaterial* srcMat ) {
  // Let the alpha channel material take care of its part
  ReAlphaChannelMaterial::fromMaterial(srcMat);
  
  auto tex = srcMat->getChannel(RE_BM_CHANNEL_NAME);
  if (!tex.isNull()) {
    channels[RE_BM_CHANNEL_NAME] = ReTexturePtr(
                                     TextureCreator::createTexture(tex->getName(), 
                                                                   tex)
                                     );    
    channels[RE_BM_CHANNEL_NAME]->reparent(this);
    addTextureToCatalog(channels[RE_BM_CHANNEL_NAME]);    
  }
  // Copy the BM values
  setBmStrength(srcMat->getNamedValue("bmStrength").toFloat());
  setBmPositive(srcMat->getNamedValue("bmPositive").toFloat());
  setBmNegative(srcMat->getNamedValue("bmNegative").toFloat());

  tex = srcMat->getChannel(RE_DM_CHANNEL_NAME);
  if (!tex.isNull()) {
    channels[RE_DM_CHANNEL_NAME] = ReTexturePtr(
                                     TextureCreator::createTexture(tex->getName(), 
                                                                   tex)
                                     );    
    channels[RE_DM_CHANNEL_NAME]->reparent(this);
    addTextureToCatalog(channels[RE_DM_CHANNEL_NAME]);
  }
  // Copy the displacement values
  setDmStrength(srcMat->getNamedValue("dmStrength").toFloat());
  setDmPositive(srcMat->getNamedValue("dmPositive").toFloat());
  setDmNegative(srcMat->getNamedValue("dmNegative").toFloat());
  // Subdivision
  setSubdivisions(srcMat->getNamedValue("subdivision").toInt());
  setKeepSharpEdges(srcMat->getNamedValue("keepSharpEdges").toBool());
  setUseMicrofacets(srcMat->getNamedValue("useMicrofacets").toBool());
  setIsSmooth(srcMat->getNamedValue("smoothnessFlag").toBool());
  // Light emission
  setEmitsLight(srcMat->getNamedValue("emitsLight").toBool());
  setLightGain(srcMat->getNamedValue("lightGain").toFloat());
  tex = srcMat->getChannel(RE_EMISSION_CHANNEL_NAME);
  if (!tex.isNull()) {
    channels[RE_EMISSION_CHANNEL_NAME] = ReTexturePtr(
      TextureCreator::createTexture(tex->getName(), tex)
    );
    channels[RE_EMISSION_CHANNEL_NAME]->reparent(this);
    addTextureToCatalog(channels[RE_EMISSION_CHANNEL_NAME]);    
  }
}


void DisplaceableMaterial::serialize( QDataStream& dataStream ) const { 
  // Serialize the base class
  ReAlphaChannelMaterial::serialize(dataStream);
  dataStream << isNormalMap << bmStrength << bmPositive << bmNegative;
  dataStream << dmStrength << dmPositive << dmNegative;

  dataStream << (quint16)subdivision << useMicrofacets 
             << smoothnessFlag << keepSharpEdgesFlag;
  // Light emission chunk
  dataStream << emitsLight << lightGain;
};

void DisplaceableMaterial::deserialize( QDataStream& dataStream ) {
  // Deserialize the base class
  ReAlphaChannelMaterial::deserialize(dataStream);

  dataStream >> isNormalMap >> bmStrength >> bmPositive >> bmNegative;
  dataStream >> dmStrength >> dmPositive >> dmNegative;
  dataStream >> subdivision >> useMicrofacets >> smoothnessFlag >> keepSharpEdgesFlag;
  // Light emission chunk
  dataStream >> emitsLight >> lightGain;
}

} // namespace
