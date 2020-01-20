/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReWaterEditor.h"
#include "textures/ReDistortedNoise.h"

ReWaterEditor::ReWaterEditor( QWidget* parent ) : ReMaterialEditor(parent) {
  setupUi(this);
  inSetup = false;
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

  // Avoid getting a "modified" event every time a character is changed in the fields
  rippleAmount->setKeyboardTracking(false);
  IOR->setKeyboardTracking(false);
  clarity->setKeyboardTracking(false);

  /*
   * Kt
   */
  connect(Kt,   SIGNAL(colorChanged()), this, SLOT(updateKt()));
  connect(rippleAmount, SIGNAL(valueChanged(double)), this, SLOT(updateRipples(double)));
  connect(clarity, SIGNAL(valueChanged(double)), this, SLOT(updateClarity(double)));
  connect(IOR, SIGNAL(valueChanged(double)), this, SLOT(updateIOR(double)));
  connect(rippleType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateRippleType(int)));
}

QSize ReWaterEditor::sizeHint() const {
  return QSize(540, 135);    
}

void ReWaterEditor::setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm ) {

  inSetup = true;

  currentMaterial = newMat;
  textureChannelModel = tcm;
  
  ReWater* mat= static_cast<ReWater*>(currentMaterial);

  rippleType->clear();
  int numRipplePresets = mat->getNumPresets();
  for (int i = 0; i < numRipplePresets; ++i) {
    rippleType->addItem(mat->getPresetName(i));
  }
  int currentPreset = mat->getPresetNumber();
  rippleType->setCurrentIndex(currentPreset);
  ripplePattern->setPixmap(QPixmap(QString(":/images/ripple_%1.png").arg(currentPreset+1)));

  Kt->setColor(mat->getKt());
  rippleAmount->setValue(mat->getRipples());
  clarity->setValue(mat->getClarityAtDepth());
  IOR->setValue(mat->getIOR());

  inSetup = false;
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));
}

void ReWaterEditor::updateKt() {
  ReWater* mat= static_cast<ReWater*>(currentMaterial);
  if (currentMaterial && !inSetup ) {
    QColor newKt = Kt->getColor();
    mat->setKt(newKt);
    mat->setEdited();
    signalUpdate("Kt", newKt);
    emit refreshVolumes();
  }  
}

void ReWaterEditor::updateRippleType(int newVal) {
  // Convert the material ptr to a convenient type
  ReWater* mat = static_cast<ReWater*>(currentMaterial);
  if (mat && !inSetup ) {
    // Obtain the data about the selected ripple preset. What it uses
    // for noise pattern and distortion. The keys are "noiseBasis"
    // and "distortionType"
    QVariantMap preset = mat->getPreset(newVal);
    QMapIterator<QString, QVariant> values(preset);
    ReTexturePtr tex = mat->getDisplacementMap();
    if (!tex.isNull()) {
      while( values.hasNext() ) {
        values.next();
        QString key = values.key();
        QVariant val = values.value();
        tex->setNamedValue(key, val);
        // signalUpdate(key, val);
        // Update the texture in the host-app side
        emit textureUpdated(tex, key, val);
      }        
    }
    mat->setNamedValue("ripplePreset", newVal);
    signalUpdate("ripplePreset", newVal);
    mat->setEdited();
    ripplePattern->setPixmap(QPixmap(QString(":/images/ripple_%1.png").arg(newVal+1)));
    emit refreshModifiers();    
  }  
} 

void ReWaterEditor::updateRipples(double newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("ripples", newVal);
    currentMaterial->setEdited();
    signalUpdate("ripples", newVal);
    emit refreshModifiers();
  }
} 

void ReWaterEditor::updateClarity(double newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("clarityAtDepth", newVal);
    currentMaterial->setEdited();
    signalUpdate("clarityAtDepth", newVal);
    emit refreshVolumes();
  }
} 

void ReWaterEditor::updateIOR(double newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("IOR", newVal);
    currentMaterial->setEdited();
    signalUpdate("IOR", newVal);
    emit refreshVolumes();
  }
} 


