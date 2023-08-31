/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
 */

#include "ReMetalEditor.h"


ReMetalEditor::ReMetalEditor( QWidget* parent ) : 
  ReMaterialEditor(parent) 
{
  setupUi(this);
  initLUT();

  // Fill the preset list with the key values from the LUT
  QMapIterator< QString, ReMetal::MetalType > i(lutMetalNameToType);
  while( i.hasNext() ) {
    i.next();
    new QListWidgetItem(i.key(), presetList);
  }

  inSetup = false;
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  // Avoid getting a "modified" event every time a character is changed in the fields
  hPolish->setKeyboardTracking(false);
  vPolish->setKeyboardTracking(false);

  // connect(metalTexture, SIGNAL(editTexture(ReTexturePtr, QString&)), 
  //         this, SIGNAL(editTexture(ReTexturePtr, QString&)));
  connect(hPolish, SIGNAL(valueChanged(int)), this, SLOT(updateHPolish(int)));
  connect(vPolish, SIGNAL(valueChanged(int)), this, SLOT(updateVPolish(int)));
  connect(presetList, SIGNAL(currentItemChanged(QListWidgetItem* , QListWidgetItem* )), 
          this, SLOT(updatePreset(QListWidgetItem*, QListWidgetItem*)));
  connect(gbCustomMetal,SIGNAL(toggled(bool)), this, SLOT(updateCustomMetal(bool)));
  connect(btnAnisotropic, SIGNAL(toggled(bool)), this, SLOT(updateAnisotropic(bool)));
  metalTexture->setDeleteEnabled(true);
}

// Initialize the Look Up Tables for metal names to types and vice versa
void ReMetalEditor::initLUT() {
  lutMetalNameToType[tr("Aluminum")] = ReMetal::Aluminum;
  lutMetalNameToType[tr("Chrome")]   = ReMetal::Chrome;
  lutMetalNameToType[tr("Cobalt")]   = ReMetal::Cobalt;
  lutMetalNameToType[tr("Copper")]   = ReMetal::Copper;
  lutMetalNameToType[tr("Gold")]     = ReMetal::Gold;
  lutMetalNameToType[tr("Lead")]     = ReMetal::Lead;
  lutMetalNameToType[tr("Mercury")]  = ReMetal::Mercury;
  lutMetalNameToType[tr("Nickel")]   = ReMetal::Nickel;
  lutMetalNameToType[tr("Platinum")] = ReMetal::Platinum;
  lutMetalNameToType[tr("Silver")]   = ReMetal::Silver;
  lutMetalNameToType[tr("Titanium")] = ReMetal::Titanium;
  lutMetalNameToType[tr("Zinc")]     = ReMetal::Zinc;

  // Initialize the reverse table
  QMapIterator<QString, ReMetal::MetalType> i(lutMetalNameToType);
  while( i.hasNext() ) {
    i.next();
    lutMetalTypeToName[i.value()] = i.key();
  }
}

QSize ReMetalEditor::sizeHint() const {
  return QSize(550, 200);    
}

void ReMetalEditor::setData( ReMaterial* newMat, 
                             ReTextureChannelDataModelPtr tcm ) 
{
  inSetup = true;

  currentMaterial = newMat;
  textureChannelModel = tcm;

  ReMetal* mat = static_cast<Reality::ReMetal*>(currentMaterial);

  ReMetal::MetalType mType = mat->getMetalType();
  if (mType == ReMetal::Custom) {
    gbCustomMetal->setChecked(true);
  }
  else {
    gbCustomMetal->setChecked(false);    
    presetList->setCurrentItem(
      presetList->findItems(lutMetalTypeToName[mType], 
                            Qt::MatchFixedString | Qt::MatchCaseSensitive)[0]
    );
  }
  metalTexture->setTexture(mat->getKr(), mat);
  hPolish->setValue(mat->getHPolish());
  vPolish->setValue(mat->getVPolish());
  if (btnAnisotropic->isChecked()) {
    vPolish->setEnabled(false);
  }
  inSetup = false;
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));

  if (teAdapter.isNull()) {
    teAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                  new ReTextureEditorAdapter(
                        this, 
                        metalTexture,
                        newMat,
                        ReTexture::ReTextureDataType::color,
                        textureChannelModel
                      )
                );
  }
  else {
    teAdapter->setMaterial(newMat);
    teAdapter->setTextureChannelModel(textureChannelModel);
  }

}

void ReMetalEditor::updateHPolish(int newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("hPolish", newVal);
    if (btnAnisotropic->isChecked()) {
      vPolish->setValue(newVal);
    }
    currentMaterial->setEdited();
    signalUpdate("hPolish", newVal);
  }
} 

void ReMetalEditor::updateVPolish(int newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("vPolish", newVal);
    currentMaterial->setEdited();
    signalUpdate("vPolish", newVal);
  }
} 


void ReMetalEditor::updatePreset( QListWidgetItem* current, QListWidgetItem* previous ) {
  if (inSetup) {
    return;
  }
  if (!current) {
    return;
  }
  QString newVal = current->text();
  currentMaterial->setNamedValue("metalType", lutMetalNameToType[newVal]);
  currentMaterial->setEdited();
  signalUpdate("metalType", currentMaterial->getNamedValue("metalType").toInt());
}


void ReMetalEditor::updateCustomMetal( bool onOff ) {
  if (onOff) {
    currentMaterial->setNamedValue("metalType", ReMetal::Custom);
    presetList->setDisabled(true);
  }
  else {
    presetList->setDisabled(false);
    if (!presetList->currentItem()) {
      presetList->setCurrentRow(0);
    }
    QString metalPreset = presetList->currentItem()->text();
    ReMetal::MetalType metalType = ReMetal::Aluminum;
    if (lutMetalNameToType.contains(metalPreset)) {
      metalType = lutMetalNameToType[metalPreset];
    }
    currentMaterial->setNamedValue("metalType", metalType);
  }
  currentMaterial->setEdited();
  signalUpdate("metalType", currentMaterial->getNamedValue("metalType").toInt());
}

void ReMetalEditor::updateAnisotropic( bool onOff ) {
  vPolish->setEnabled(!onOff);
}
