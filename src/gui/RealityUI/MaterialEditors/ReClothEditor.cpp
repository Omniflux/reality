/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReClothEditor.h"

using namespace Reality;

ReClothEditor::ReClothEditor( QWidget* parent ) : ReMaterialEditor(parent) {
  setupUi(this);
  currentMaterial = NULL;
  inSetup = false;
  setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

  for (int i = 0; i < RE_CLOTH_NUM_PRESETS; ++i) {
    preset->addItem(ReCloth::clothPresets[i].label);
  }

  uRepeat->setKeyboardTracking(false);
  vRepeat->setKeyboardTracking(false);
  connect(btnApplyPreset, SIGNAL(clicked()),this,   SLOT(updatePreset()));

  warpKdAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                    new ReTextureEditorAdapter(
                          this, 
                          warp_Kd, 
                          currentMaterial, 
                          ReTexture::ReTextureDataType::color,
                          textureChannelModel
                        )
                  );
  warpKsAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                    new ReTextureEditorAdapter(
                          this, 
                          warp_Ks, 
                          currentMaterial, 
                          ReTexture::ReTextureDataType::color,
                          textureChannelModel
                        )
                  );
  weftKdAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                    new ReTextureEditorAdapter(
                          this, 
                          weft_Kd, 
                          currentMaterial, 
                          ReTexture::ReTextureDataType::color,
                          textureChannelModel
                        )
                  );
  weftKsAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                    new ReTextureEditorAdapter(
                          this, 
                          weft_Ks, 
                          currentMaterial, 
                          ReTexture::ReTextureDataType::color,
                          textureChannelModel
                        )
                  );

  connect(uRepeat, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(vRepeat, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
}

QSize ReClothEditor::sizeHint() const {
  return QSize(673, 205);    
}

void ReClothEditor::setData( ReMaterial* newMat, 
                             ReTextureChannelDataModelPtr tcm ) 
{
  inSetup = true;

  currentMaterial = newMat;
  textureChannelModel = tcm;
  
  ReCloth* mat = static_cast<ReCloth*>(currentMaterial);
  preset->setCurrentIndex(mat->getPresetNumber());
  warp_Kd->setTexture(mat->getWarp_Kd(), mat);
  warp_Ks->setTexture(mat->getWarp_Ks(), mat);
  weft_Kd->setTexture(mat->getWeft_Kd(), mat);
  weft_Ks->setTexture(mat->getWeft_Ks(), mat);

  warpKdAdapter->setMaterial(newMat);
  warpKdAdapter->setTextureChannelModel(tcm);

  warpKsAdapter->setMaterial(newMat);
  warpKsAdapter->setTextureChannelModel(tcm);

  weftKdAdapter->setMaterial(newMat);
  weftKdAdapter->setTextureChannelModel(tcm);

  weftKsAdapter->setMaterial(newMat);
  weftKsAdapter->setTextureChannelModel(tcm);

  uRepeat->setValue(mat->getURepeat());
  vRepeat->setValue(mat->getVRepeat());
  inSetup = false;
}

void ReClothEditor::updatePreset() {
  if (inSetup) {
    return;
  }
  int presetIndex = preset->currentIndex();
  auto presetData = ReClothPreset::getPreset(presetIndex);

  if (cbOverwriteTextures->isChecked()) {
    currentMaterial->setNamedValue("preset", presetIndex);
    signalUpdate("preset",  presetIndex);
  }
  else {
    currentMaterial->setNamedValue("uRepeat", presetData->uRepeat);
    currentMaterial->setNamedValue("vRepeat", presetData->vRepeat);
    currentMaterial->setNamedValue("presetName", presetData->name);
    signalUpdate("uRepeat", presetData->uRepeat);
    signalUpdate("vRepeat", presetData->vRepeat);
    signalUpdate("presetName",  presetData->name);
  }
  setData(currentMaterial, textureChannelModel);
  currentMaterial->setEdited();  
}

void ReClothEditor::updateValue( double newVal ) {
  if (inSetup) {
    return;
  }
  QString source = QObject::sender()->objectName();
  currentMaterial->setNamedValue(source, newVal);
  currentMaterial->setEdited();
  signalUpdate(source, newVal);
} 

