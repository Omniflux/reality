/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
 */

#include "ReGlossyEditor.h"
#include "ReGlossy.h"

using namespace Reality;

ReGlossyEditor::ReGlossyEditor( QWidget* parent ) :
  ReMaterialEditor(parent) 
{
  setupUi(this);
  inSetup = false;
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

  // Avoid getting a "modified" event every time a character is changed in the fields
  uGlossiness->setKeyboardTracking(false);
  vGlossiness->setKeyboardTracking(false);
  coatThickness->setKeyboardTracking(false);
  // Enable the Delete menu for the Kg texture
  glossyKg->setDeleteEnabled(true);
  glossyKt->setDeleteEnabled(true);

  KdAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      glossyKd, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KsAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      glossyKs, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KaAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      glossyKa, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KtAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      glossyKt, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KgAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      glossyKg, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::numeric,
                      textureChannelModel
                    )
              );

  // Specular must be a color texture
  glossyKs->setAllowedDataTypes(RE_TEX_DATATYPE_COLOR);

  // Kt must be a color texture
  glossyKt->setAllowedDataTypes(RE_TEX_DATATYPE_COLOR);

  // Kg must be a float texture
  glossyKg->setAllowedDataTypes(RE_TEX_DATATYPE_NUMERIC);

  connect(uGlossiness,      SIGNAL(valueChanged(int)),    this, SLOT(updateUGlossiness(int)));
  connect(vGlossiness,      SIGNAL(valueChanged(int)),    this, SLOT(updateVGlossiness(int)));
  connect(coatThickness,    SIGNAL(valueChanged(double)), this, SLOT(updateCoatThickness(double)));
  connect(surfaceFuzz,      SIGNAL(toggled(bool)),        this, SLOT(updateSurfaceFuzz(bool)));
  connect(btnAnisotropic,   SIGNAL(toggled(bool)),        this, SLOT(updateAnisotropic(bool)));
  connect(gbTranslucency,   SIGNAL(toggled(bool)),        this,  SLOT(updateTranslucency(bool)));
  connect(gbCoat,           SIGNAL(toggled(bool)),        this,  SLOT(updateTopCoat(bool)));

}

QSize ReGlossyEditor::sizeHint() const {
  return QSize(630, 450);    
}

void ReGlossyEditor::setData( ReMaterial* newMat, 
                              ReTextureChannelDataModelPtr tcm ) 
{
  inSetup = true;

  currentMaterial = newMat;
  textureChannelModel = tcm;
  
  Glossy* mat = static_cast<Glossy*>(currentMaterial);

  KdAdapter->setMaterial(newMat);
  KdAdapter->setTextureChannelModel(tcm);

  KsAdapter->setMaterial(newMat);
  KsAdapter->setTextureChannelModel(tcm);

  KaAdapter->setMaterial(newMat);
  KaAdapter->setTextureChannelModel(tcm);

  KtAdapter->setMaterial(newMat);
  KtAdapter->setTextureChannelModel(tcm);

  KgAdapter->setMaterial(newMat);
  KgAdapter->setTextureChannelModel(tcm);

  glossyKd->setTexture(mat->getKd(), mat);
  glossyKs->setTexture(mat->getKs(), mat);
  glossyKa->setTexture(mat->getKa(), mat);
  glossyKt->setTexture(mat->getKt(), mat);
  uGlossiness->setValue(mat->getUGlossiness());
  vGlossiness->setValue(mat->getVGlossiness());

  btnAnisotropic->setChecked(mat->getUGlossiness() == mat->getVGlossiness());
  updateAnisotropic(btnAnisotropic->isChecked());

  glossyKg->setTexture(mat->getKg(), mat);

  gbCoat->setChecked(mat->hasTopCoat());
  gbTranslucency->setChecked(mat->isTranslucent());
  coatThickness->setValue(mat->getCoatThickness());
  surfaceFuzz->setChecked(mat->getSurfaceFuzz());

  inSetup = false;
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));
}

void ReGlossyEditor::updateUGlossiness(int newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("uGlossiness",newVal);
    signalUpdate("uGlossiness", newVal);
    currentMaterial->setEdited();
    if (btnAnisotropic->isChecked()) {
      vGlossiness->setValue(newVal);      
    }
  }
} 

void ReGlossyEditor::updateVGlossiness(int newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("vGlossiness",newVal);
    currentMaterial->setEdited();
    signalUpdate("vGlossiness", newVal);
  }
} 

void ReGlossyEditor::updateCoatThickness(double newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("coatThickness",newVal);
    currentMaterial->setEdited();
    signalUpdate("coatThickness", newVal);
  }
} 

void ReGlossyEditor::updateMixLevel(double newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("mixLevel",newVal);
    currentMaterial->setEdited();
    signalUpdate("mixLevel", newVal);
  }
} 

void ReGlossyEditor::updateSurfaceFuzz( bool newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("surfaceFuzz",newVal);
    currentMaterial->setEdited();
    signalUpdate("surfaceFuzz", newVal);
  }
}

void ReGlossyEditor::updateTranslucency( bool newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("translucent",newVal);
    currentMaterial->setEdited();
    signalUpdate("translucent", newVal);
  }
}

void ReGlossyEditor::updateTopCoat( bool newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("topCoat",newVal);
    currentMaterial->setEdited();
    signalUpdate("topCoat", newVal);
  }
}


void ReGlossyEditor::updateAnisotropic( bool onOff ) {
  if (onOff) {
    vGlossiness->setValue(uGlossiness->value());
  }
  vGlossiness->setEnabled(!onOff);
}

