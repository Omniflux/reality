/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

//! \file
  
#include "ReCloth.h"

#include "ReTextureCreator.h"
#include "textures/ReConstant.h"


namespace Reality {

// Static variable initialization
ReClothPreset ReCloth::clothPresets[RE_CLOTH_NUM_PRESETS] = {
  ReClothPreset("denim", 
                "Denim", 
                QColor(2,   3,  4),
                QColor(5,   7,  8),
                QColor(35, 33, 41),
                QColor(90, 90, 90),
                117.0f,
                58.5f),
  ReClothPreset("silk_charmeuse", 
                "Silk Charmeuse", 
                QColor(17,  0,  0),
                QColor(70, 12, 10),
                QColor(17,  0,  0),
                QColor(69, 12, 10),
                200.0f,
                100.0f),
  ReClothPreset("silk_shantung", 
                "Silk Shantung", 
                QColor(2,   1,  1),
                QColor(15, 16, 17),
                QColor(11,  1,  1),
                QColor(66, 23, 21),
                224.0f,
                168.0f),
  ReClothPreset("cotton_twill", 
                "Cotton Twill", 
                QColor(18, 14, 12),
                QColor(19, 19, 19),
                QColor(18, 12, 14),
                QColor(19, 19, 19),
                125.0f,
                62.5f),
  ReClothPreset("wool_garbardine", 
                "Wool Gabardine", 
                QColor(75, 12,  5),
                QColor(85, 30, 23),
                QColor(75, 12,  5),
                QColor(85, 30, 23),
                100.8f,
                67.2f),
  ReClothPreset("polyester_lining_cloth", 
                "Polyester Lining", 
                QColor(61,  9,  5),
                QColor(27, 15, 13),
                QColor(61,  9,  5),
                QColor(27, 15, 13),
                225.0f,
                225.0f)
};

ReClothPreset* ReClothPreset::getPreset( unsigned short index ) {
  if ( index > RE_CLOTH_NUM_PRESETS ) {
    index = 0;
  }
  return &ReCloth::clothPresets[index];
}

ReCloth::ReCloth( const QString name, const ReGeometryObject* parent ) :
  DisplaceableMaterial(name, parent)
{
  type = originalType = MatCloth;
  ReClothPreset thePreset = clothPresets[RE_CLOTH_DENIM];
  presetName = thePreset.name;

  channels[WARP_KD] = ReTexturePtr(
    new ReConstant(
          WARP_KD,
          this,
          thePreset.warp_Kd
    )
  );
  channels[WARP_KS] =  ReTexturePtr(
    new ReConstant(
          WARP_KS,
          this,
          thePreset.warp_Ks
    )
  );
  channels[WEFT_KD] = ReTexturePtr(
    new ReConstant(
          WEFT_KD,
          this,
          thePreset.weft_Kd
    )
  );
  channels[WEFT_KS] = ReTexturePtr(
    new ReConstant(
          WEFT_KS,
          this,
          thePreset.weft_Ks
    )
  );

  addTextureToCatalog(channels[WARP_KD]);
  addTextureToCatalog(channels[WARP_KS]);
  addTextureToCatalog(channels[WEFT_KD]);
  addTextureToCatalog(channels[WEFT_KS]);

  channels[WARP_KD]->reparent(this);
  channels[WARP_KS]->reparent(this);
  channels[WEFT_KD]->reparent(this);
  channels[WEFT_KS]->reparent(this);

  uRepeat = thePreset.uRepeat;
  vRepeat = thePreset.vRepeat;

  ReNodeDictionaryIterator c(channels);
};

void ReCloth::fromMaterial( const ReMaterial* baseMat ) {
  DisplaceableMaterial::fromMaterial(baseMat);
  auto tex = baseMat->getChannel("Kd");
  if (!tex.isNull()) {
    deleteTexture(channels[WARP_KD]->getName());
    channels[WARP_KD].clear();
    channels[WARP_KD] = ReTexturePtr(
                          TextureCreator::createTexture(
                            QString("%1_warp").arg(tex->getName()), 
                            tex
                          )
                        );
    channels[WARP_KD]->reparent(this);
    addTextureToCatalog(channels[WARP_KD]);

    deleteTexture(channels[WEFT_KD]->getName());
    channels[WEFT_KD].clear();
    channels[WEFT_KD] = ReTexturePtr(
                          TextureCreator::createTexture(
                            QString("%1_weft").arg(tex->getName()), 
                            tex
                          )
                        );
    channels[WEFT_KD]->reparent(this);
    addTextureToCatalog(channels[WEFT_KD]);
  }
  
  tex = baseMat->getChannel("Ks");
  if (!tex.isNull()) {
    deleteTexture(channels[WARP_KS]->getName());
    channels[WARP_KS].clear();
    channels[WARP_KS] = ReTexturePtr(
                          TextureCreator::createTexture(
                            QString("%1_warp").arg(tex->getName()),
                            tex
                          )
                        );
    channels[WARP_KS]->reparent(this);
    addTextureToCatalog(channels[WARP_KS]);

    deleteTexture(channels[WEFT_KS]->getName());
    channels[WEFT_KS].clear();
    channels[WEFT_KS] = ReTexturePtr(
                          TextureCreator::createTexture(
                            QString("%1_weft").arg(tex->getName()),
                            tex
                          )
                        );
    channels[WEFT_KS]->reparent(this);
    addTextureToCatalog(channels[WEFT_KS]);
  }
}

// Destructor: ReCloth
ReCloth::~ReCloth() {
};

const QString ReCloth::getPresetName() const {
  return presetName;
};

int ReCloth::getPresetNumber() {
  for (int i = 0; i < RE_CLOTH_NUM_PRESETS; ++i) {
    if ( presetName == clothPresets[i].name ) {
      return i;
    }
  }
  return 0;
}

void ReCloth::setPresetName( const QString& newVal ) {
  presetName = newVal;
};

void ReCloth::setPreset( const int presetIndex ) {
  ReClothPreset& newPreset = clothPresets[presetIndex];

  presetName = newPreset.name;
  setWarp_Kd(
    ReTexturePtr(
      new ReConstant(WARP_KD, this, newPreset.warp_Kd)
    )
  );
  setWarp_Ks(
    ReTexturePtr(
      new ReConstant(WARP_KS, this, newPreset.warp_Ks)
    )
  );
  setWeft_Kd(
    ReTexturePtr(
      new ReConstant(WEFT_KD, this, newPreset.weft_Kd)
    )
  );
  setWeft_Ks(
    ReTexturePtr(
      new ReConstant(WEFT_KS, this, newPreset.weft_Ks)
    )
  );
  setURepeat(newPreset.uRepeat);
  setVRepeat(newPreset.vRepeat);  
}


const ReTexturePtr ReCloth::getWarp_Kd() const {
  return channels[WARP_KD];
};

void ReCloth::setWarp_Kd( ReTexturePtr newVal ) {
  // Clear the channel...
  setChannel(WARP_KD, "");
  newVal->reparent(this);
  addTextureToCatalog(newVal);  
  setChannel(WARP_KD, newVal->getName());
};

const ReTexturePtr ReCloth::getWarp_Ks() const {
  return channels[WARP_KS];
};
void ReCloth::setWarp_Ks( ReTexturePtr newVal ) {
  // Clear the channel...
  setChannel(WARP_KS, "");
  newVal->reparent(this);
  addTextureToCatalog(newVal);  
  setChannel(WARP_KS, newVal->getName());
};

const ReTexturePtr ReCloth::getWeft_Kd() const {
  return channels[WEFT_KD];
};
void ReCloth::setWeft_Kd( ReTexturePtr newVal ) {
  // Clear the channel...
  setChannel(WEFT_KD, "");
  newVal->reparent(this);
  addTextureToCatalog(newVal);  
  setChannel(WEFT_KD, newVal->getName());
};

const ReTexturePtr ReCloth::getWeft_Ks() const {
  return channels[WEFT_KS];
};

void ReCloth::setWeft_Ks( ReTexturePtr newVal ) {
  // Clear the channel...
  setChannel(WEFT_KS, "");
  newVal->reparent(this);
  addTextureToCatalog(newVal);  
  setChannel(WEFT_KS, newVal->getName());
};


void ReCloth::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "presetName") {
    presetName = value.toString();
  }
  if (vname == "preset") {
    setPreset(value.toInt());
  }
  else if ( vname == "uRepeat" ) {
    uRepeat = value.toFloat();
  }
  else if ( vname == "vRepeat" ) {
    vRepeat = value.toFloat();
  }
  else if ( vname == WARP_KD || vname == WARP_KS || 
            vname == WEFT_KD || vname == WEFT_KS) {
    ReTexturePtr tex = value.value<ReTexturePtr>();
    tex->reparent(this);
    addTextureToCatalog(tex);
    // Clear the channel...
    setChannel(vname, "");
    setChannel(vname,tex->getName());
  }
  else {
    DisplaceableMaterial::setNamedValue(vname, value);
  }
};

const QVariant ReCloth::getNamedValue( const QString& vname ) const {
  QVariant val;
  if (vname == "presetName") {
    return presetName;
  }
  else if ( vname == "uRepeat" ) {
    return uRepeat;
  }
  else if ( vname == "vRepeat" ) {
    return vRepeat;
  }
  else if ( vname == WARP_KD ) {
    val.setValue(channels[WARP_KD]);
    return val;
  }
  else if ( vname == WARP_KS ) {
    val.setValue(channels[WARP_KS]);
    return val;
  }
  else if ( vname == WEFT_KD ) {
    val.setValue(channels[WEFT_KD]);
    return val;
  }
  else if ( vname == WEFT_KS ) {
    val.setValue(channels[WEFT_KS]);
    return val;
  }
  return DisplaceableMaterial::getNamedValue(vname);
};

void ReCloth::serialize( QDataStream& dataStream ) const {
  // Call the base class serializer
  ReMaterial::serialize(dataStream);
  dataStream << presetName << uRepeat << vRepeat;
  DisplaceableMaterial::serialize(dataStream);  
}


void ReCloth::deserialize( QDataStream& dataStream ){
  ReMaterial::deserialize( dataStream );
  dataStream >> presetName >> uRepeat >> vRepeat;
  DisplaceableMaterial::deserialize(dataStream);
}


} // namespace
