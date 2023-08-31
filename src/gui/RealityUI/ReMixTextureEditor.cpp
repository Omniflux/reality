/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReMixTextureEditor.h"

#include "ReMaterial.h"
#include "ReTexture.h"
#include "ReTextureChannelDataModel.h"
#include "RealityUI/ReTextureEditorAdapter.h"


ReMixTextureEditor::ReMixTextureEditor(QWidget* parent) : 
  ReAbstractTextureEditor(parent) 
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);


  uiUpdating = false;

  // Avoid getting a "modified" event every time a character is changed in the fields
  mixAmount->setKeyboardTracking(false);
  mixTexture->setDeleteEnabled(true);
}

QSize ReMixTextureEditor::sizeHint() const {
  return QSize(260, 286);
}



void ReMixTextureEditor::setDataModel( Reality::ReTextureChannelDataModelPtr _model, 
                                       ReMaterial* mat ) {
  ReAbstractTextureEditor::setDataModel(_model, mat);

  auto baseTexture = model->getTexture();
  auto dataType = baseTexture->getDataType();

  if (Tex1Adapter.isNull()) {
    Tex1Adapter = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          texture1, 
                          material, 
                          dataType,
                          _model,
                          baseTexture
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
                          dataType,
                          _model,
                          baseTexture
                        )
                  );
  }
  else {
    Tex2Adapter->setMaterial(material);    
    Tex2Adapter->setMasterTexture(model->getTexture());
  }
  if (MixAdapter.isNull()) {  
    MixAdapter = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          mixTexture, 
                          material, 
                          dataType,
                          _model,
                          baseTexture
                        )
                  );
  }
  else {
    MixAdapter->setMaterial(material);    
    MixAdapter->setMasterTexture(model->getTexture());
  }

  refreshUI();

  // Editing of the first texture...
  connect(texture1, SIGNAL(editTexture( ReTexturePtr, QString& )), 
          this,     SIGNAL(editTexture( ReTexturePtr, QString& )));
  texture1->setDeleteEnabled(true);
  // Editing of the second texture...
  connect(texture2, SIGNAL(editTexture( ReTexturePtr, QString& )), 
          this,     SIGNAL(editTexture( ReTexturePtr, QString& )));
  texture2->setDeleteEnabled(true);

  // Editing of the mix texture...
  connect(mixTexture, SIGNAL(editTexture( ReTexturePtr, QString& )), 
          this,       SIGNAL(editTexture( ReTexturePtr, QString& )));
  connect(mixAmount, SIGNAL(valueChanged(double)), this, SLOT(updateMixAmount(double)));

  connect(model.data(), SIGNAL(modelChanged()), SLOT(refreshUI()));
}

void ReMixTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  texture1->setTexture(model->getNamedValue("texture1").value<ReTexturePtr>(), material);
  texture2->setTexture(model->getNamedValue("texture2").value<ReTexturePtr>(), material);
  mixAmount->setValue(model->getNamedValue("mixAmount").toDouble());
  mixTexture->setTexture(model->getNamedValue("mixTexture").value<ReTexturePtr>(), material);
}

void ReMixTextureEditor::updateMixAmount(double newVal) {
  model->setNamedValue("mixAmount", newVal);
}


