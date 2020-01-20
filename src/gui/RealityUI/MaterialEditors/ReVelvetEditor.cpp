/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReVelvetEditor.h"

using namespace Reality;

ReVelvetEditor::ReVelvetEditor( QWidget* parent ) : 
  ReMaterialEditor(parent) 
{
  setupUi(this);
  currentMaterial = NULL;
  inSetup = false;
  setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

  thickness->setKeyboardTracking(false);

  KdAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      Kd, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );

  connect(thickness, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
}

QSize ReVelvetEditor::sizeHint() const {
  return QSize(410, 85);    
}

void ReVelvetEditor::setData( ReMaterial* newMat, 
                              ReTextureChannelDataModelPtr tcm ) 
{
  inSetup = true;

  currentMaterial = newMat;
  textureChannelModel = tcm;

  Kd->setTexture(currentMaterial->getNamedValue("Kd").value<ReTexturePtr>(), currentMaterial);

  KdAdapter->setMaterial(newMat);
  KdAdapter->setTextureChannelModel(tcm);
  
  thickness->setValue(currentMaterial->getNamedValue("thickness").toFloat());
  inSetup = false;
}

void ReVelvetEditor::updateValue( double newVal ) {
  if (inSetup) {
    return;
  }
  currentMaterial->setNamedValue("thickness", newVal);
  currentMaterial->setEdited();
  signalUpdate("thickness", newVal);
} 

