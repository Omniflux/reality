/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReMathTextureEditor.h"

#include "RealityUI/ReTextureEditorAdapter.h"
#include "textures/ReMath.h"


ReMathTextureEditor::ReMathTextureEditor(QWidget* parent) : 
  ReAbstractTextureEditor(parent) 
{
  setupUi(this);
  inSetup = false;
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

  // Editing of the first texture...
  connect(texture1, SIGNAL(editTexture( ReTexturePtr, QString&)), 
          this,     SIGNAL(editTexture( ReTexturePtr, QString&)));

  // Editing of the second texture...
  connect(texture2, SIGNAL(editTexture( ReTexturePtr, QString&)), 
          this,     SIGNAL(editTexture( ReTexturePtr, QString&)));

  connect(amount1, SIGNAL(valueChanged(double)), this, SLOT(updateAmount1(double)));
  connect(amount2, SIGNAL(valueChanged(double)), this, SLOT(updateAmount2(double)));
  connect(function, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFunction(int)));
}

QSize ReMathTextureEditor::sizeHint() const {
  return QSize(300,534);
}

void ReMathTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  inSetup = true;

  model = _model;
  material = mat;

  if (Tex1Adapter.isNull()) {
    Tex1Adapter = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          texture1, 
                          material, 
                          ReTexture::ReTextureDataType::numeric,
                          _model,
                          model->getTexture()
                        )
                  );
  }
  else {
    Tex1Adapter->setMaterial(material);
    Tex1Adapter->setMasterTexture(model->getTexture());
  }
  if (Tex2Adapter.isNull()) {  
    Tex2Adapter = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          texture2, 
                          material, 
                          ReTexture::ReTextureDataType::numeric,
                          _model,
                          model->getTexture()
                        )
                  );
  }
  else {
    Tex2Adapter->setMaterial(material);    
    Tex2Adapter->setMasterTexture(model->getTexture());
  }

  amount1->setValue(model->getNamedValue("amount1").toFloat());
  amount2->setValue(model->getNamedValue("amount2").toFloat());
  texture1->setTexture(model->getNamedValue("texture1").value<ReTexturePtr>(), material);
  texture2->setTexture(model->getNamedValue("texture2").value<ReTexturePtr>(), material);
  function->setCurrentIndex(model->getNamedValue("function").toInt());
  inSetup = false;
}

void ReMathTextureEditor::updateAmount1(double newVal) {
  if (inSetup) {
    return;
  }
  model->setNamedValue("amount1", newVal);
}

void ReMathTextureEditor::updateAmount2(double newVal) {
  if (inSetup) {
    return;
  }
  model->setNamedValue("amount2", newVal);
}

void ReMathTextureEditor::updateFunction(int newVal) {
  if (inSetup) {
    return;
  }
  model->setNamedValue("function", static_cast<ReMath::functions>(newVal));
}
