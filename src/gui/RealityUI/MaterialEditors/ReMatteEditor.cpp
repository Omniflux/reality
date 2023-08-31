/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReMatte.h"
#include "ReMatteEditor.h"


ReMatteEditor::ReMatteEditor( QWidget* parent ) : 
  ReMaterialEditor(parent) 
{
  setupUi(this);
  currentMaterial = NULL;
  inSetup = false;
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  KdAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      diffuse, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KtAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      Kt, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );

  connect(roughness, SIGNAL(valueChanged(double)), this, SLOT(updateRoughness(double)));
  connect(gbTranslucency, SIGNAL(toggled(bool)), this, SLOT(updateTranslucency(bool)));
  connect(exposureControl, SIGNAL(toggled(bool)), this, SLOT(updateConserveEnergy(bool)));
}

QSize ReMatteEditor::sizeHint() const {
  return QSize(700, 72);    
}

void ReMatteEditor::setData( ReMaterial* newMat, 
                             ReTextureChannelDataModelPtr tcm ) 
{
  inSetup = true;

  currentMaterial = static_cast<Reality::ReMatte*>(newMat);
  textureChannelModel = tcm;

  diffuse->setTexture(currentMaterial->getKd(), currentMaterial);
  Kt->setTexture(currentMaterial->getKt(), currentMaterial);
  roughness->setValue(currentMaterial->getRoughness());
  gbTranslucency->setChecked(currentMaterial->isTranslucent());
  exposureControl->setChecked(currentMaterial->conservesEnergy());

  KdAdapter->setMaterial(newMat);
  KdAdapter->setTextureChannelModel(tcm);

  KtAdapter->setMaterial(newMat);
  KtAdapter->setTextureChannelModel(tcm);

  inSetup = false;
}

void ReMatteEditor::updateRoughness(double newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setRoughness(newVal);
    currentMaterial->setEdited();
    signalUpdate("roughness", newVal);
  }
} 

void ReMatteEditor::updateTranslucency( bool onOff ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setTranslucent(onOff);
    signalUpdate("translucent", onOff);
  }
}

void ReMatteEditor::updateConserveEnergy( bool onOff ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setConserveEnergy(onOff);
    signalUpdate("conserveEnergy", onOff);
  }
}

void ReMatteEditor::signalUpdate(QString valueName, QVariant value) {
  emit materialUpdated( currentMaterial->getParent()->getInternalName(), 
                        currentMaterial->getName(),
                        valueName,
                        value );
}
