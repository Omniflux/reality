/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReSkin.h"

#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReTextureCreator.h"


#if defined(_MSC_VER)
  // Disable endless warning about truncation with VC++
  #pragma warning (disable:4305)
#endif

namespace Reality {

ReSkin::ReSkin( const QString name, const ReGeometryObject* parent ) :
  ReGlossy(name, parent)
{
  init();
}

void ReSkin::fromMaterial( const ReMaterial* baseMat ) {
  // Most of the work is done by the superclass
  ReGlossy::fromMaterial(baseMat);
  if (baseMat->getType() == MatGlossy) {
    auto tex = baseMat->getChannel(RE_GLOSSY_KT_CH);
    if (!tex.isNull() && baseMat->getNamedValue("translucent").toBool()) {
      channels[RE_GLOSSY_KA_CH] = ReTexturePtr(ReTextureCreator::createTexture(tex->getName(), tex));
      channels[RE_GLOSSY_KA_CH]->reparent(this);
      addTextureToCatalog(channels[RE_GLOSSY_KA_CH]);
      setTopCoat(true);
    }    
  }
  channels[RE_SKIN_KHM] = channels["Kd"];
}

void ReSkin::init() {
  type = originalType = MatSkin;

  uGlossiness   = vGlossiness = 5000;
  fresnelAmount = 0.05f;
  surfaceFuzz   = false;
  sssEnabled    = true;
  hairMask      = false;
  hmGain        = 1.4;

  // The Ka and Kt channels are not used with skin and they are 
  // initialized by the base class. So we removed them.
  // setChannel("Ka", "");
  setChannel("Kt", "");
  // channels.remove("Ka");
  channels.remove("Kt");
  // Initializes the Hair mask channel
  channels[RE_SKIN_KHM] = ReTexturePtr();
  createSSSVolume();
}

void ReSkin::setHairMask( bool onOff ) {
  hairMask = onOff;
  if (hairMask) {
    // if (!channels.contains(RE_SKIN_KHM)) {
    if (channels.value(RE_SKIN_KHM).isNull()) {
      channels[RE_SKIN_KHM] = channels["Kd"];
    }
  }
  else {
    if (channels.contains(RE_SKIN_KHM)) {
      channels.remove(RE_SKIN_KHM);
    }
  }
}


void ReSkin::createSSSVolume() {
  // For SSS
  QString objID = parent->getInternalName();
  QString volumeName = innerVolume.isNull() ? 
                         QString(RE_SKIN_VOL_TEMPLATE).arg(objID) : 
                         innerVolume;
                         
  if (!RealitySceneData->volumeExists(volumeName)) {
    ReVolumePtr vol = ReVolumePtr( new ReVolume() );
    vol->setType(ReVolume::homogeneous);
    vol->setName(volumeName);
    vol->setColor(RE_SKIN_ABSORTION_COLOR);
    vol->setAbsorptionScale(RE_SKIN_ABSORTION_SCALE);
    vol->setScatteringColor(RE_SKIN_SCATTERING_COLOR);
    vol->setScatteringScale(RE_SKIN_SCATTERING_SCALE);
    vol->setIOR(RE_SKIN_IOR);
    vol->setClarityAtDepth(RE_SKIN_CLARITY_AT_DEPTH);
    vol->setDirectionR(RE_SKIN_DIRECTION_RGB);
    vol->setDirectionG(RE_SKIN_DIRECTION_RGB);
    vol->setDirectionB(RE_SKIN_DIRECTION_RGB);
    vol->setEditable(false);
    RealitySceneData->saveVolume(vol);
  }  
  RealitySceneData->linkVolumeToMaterial(volumeName, objID, name);
  innerVolume = volumeName;
}

/*
 Method: serialize      
*/
void ReSkin::serialize( QDataStream& dataStream ) const { 
  // Call the base class serializer
  ReGlossy::serialize(dataStream);
  dataStream << fresnelAmount     
             << sssEnabled             
             << hairMask
             << hmGain;

  DisplaceableMaterial::serialize(dataStream);      
};

void ReSkin::deserialize( QDataStream& dataStream ) {
  ReGlossy::deserialize( dataStream );
  dataStream >> fresnelAmount   
             >> sssEnabled             
             >> hairMask
             >> hmGain;

  DisplaceableMaterial::deserialize(dataStream);      
}


void ReSkin::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "fresnelAmount") {
    fresnelAmount = value.toDouble();
  }
  else if (vname == "sssEnabled") {
    setSSSEnabled(value.toBool());
  }
  else if (vname == "hairMask") {
    setHairMask(value.toBool());
  }
  else if (vname == "hmGain") {
    hmGain = value.toFloat();
  }
  else if (vname == RE_SKIN_KHM) {
    setKhm(value.value<ReTexturePtr>());
  }
  else {
    ReGlossy::setNamedValue(vname, value);
  }
}

const QVariant ReSkin::getNamedValue( const QString& vname ) const {
  QVariant val;
  if (vname == "fresnelAmount") {
    return fresnelAmount;
  }
  else if (vname == "sssEnabled") {
    return sssEnabled;
  }
  else if (vname == "hairMask") {
    return hairMask;
  }
  else if (vname == "hmGain") {
    return hmGain;
  }
  else if (vname == RE_SKIN_KHM) {
    val.setValue(channels[RE_SKIN_KHM]);
    return val;
  }
  else {
    return ReGlossy::getNamedValue(vname);
  }
  return val;
}

QString ReSkin::toString() {
  return QString("Skin: %1, SSS:%2, inner vol:%3, outer vol:%4")
           .arg(name)
           .arg(sssEnabled? "Yes" : "No")
           .arg(innerVolume)
           .arg(outerVolume);
}


} // namespace
