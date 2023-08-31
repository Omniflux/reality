/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
 */

#include "ReAlphaChannelEditor.h"

#include "ReAlphaChannelMaterial.h"


using namespace Reality;

ReAlphaChannelEditor::ReAlphaChannelEditor( QWidget* parent ) :
  ReMaterialEditor(parent) 
{
  setupUi(this);
  inSetup = false;

  // Enable the Delete menu for the Kg texture
  alphaMap->setDeleteEnabled(true);

  alphaAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                   new ReTextureEditorAdapter(
                         this, 
                         alphaMap, 
                         currentMaterial, 
                         ReTexture::ReTextureDataType::numeric,
                         textureChannelModel
                       )
                 );

  // Specular must be a color texture
  alphaMap->setAllowedDataTypes(RE_TEX_DATATYPE_NUMERIC);

  connect(alphaStrength,    SIGNAL(valueChanged(double)), this, SLOT(updateAlphaValue(double)));
}

QSize ReAlphaChannelEditor::sizeHint() const {
  return QSize(495, 85);    
}

void ReAlphaChannelEditor::setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm ) {
  inSetup = true;

  currentMaterial = newMat;
  textureChannelModel = tcm;

  ReAlphaChannelMaterial* mat = static_cast<ReAlphaChannelMaterial*>(currentMaterial);
  
  alphaAdapter->setMaterial(newMat);
  alphaAdapter->setTextureChannelModel(tcm);

  alphaMap->setTexture(mat->getAlphaMap(), mat);
  alphaStrength->setValue(mat->getAlphaStrength());

  inSetup = false;
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));
}

void ReAlphaChannelEditor::updateAlphaValue(double newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("alphaStrength",newVal);
    signalUpdate("alphaStrength", newVal);
    currentMaterial->setEdited();
  }
} 
