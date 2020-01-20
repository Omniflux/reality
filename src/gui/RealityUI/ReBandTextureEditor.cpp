/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/


#include "ReBandTextureEditor.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"

ReBandTextureEditor::ReBandTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent) 
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

  // Avoid getting a "modified" event every time a character is changed in the fields
  offset1->setKeyboardTracking(false);
  offset2->setKeyboardTracking(false);
  offset3->setKeyboardTracking(false);
  offset4->setKeyboardTracking(false);

  bandMap->setDeleteEnabled(true);
  uiUpdating = false;
}

QSize ReBandTextureEditor::sizeHint() const{
  return QSize(325, 345);
}

void ReBandTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, 
                                        ReMaterial* mat ) 
{
  disconnect(offset1,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  disconnect(offset2,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  disconnect(offset3,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  disconnect(offset4,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  disconnect(amount,    SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );

  ReAbstractTextureEditor::setDataModel(_model, mat);
  refreshUI();
  if (tex1Adtp.isNull()) {
    tex1Adtp = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          tex1, 
                          material, 
                          ReTexture::ReTextureDataType::color,
                          _model,
                          model->getTexture()
                        )
                  );
  }
  else {
    tex1Adtp->setMaterial(material);
    tex1Adtp->setMasterTexture(model->getTexture());
  }
  if (tex2Adtp.isNull()) {
    tex2Adtp = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          tex2, 
                          material, 
                          ReTexture::ReTextureDataType::color,
                          _model,
                          model->getTexture()
                        )
                  );
  }
  else {
    tex2Adtp->setMaterial(material);
    tex2Adtp->setMasterTexture(model->getTexture());
  }
  if (tex3Adtp.isNull()) {
    tex3Adtp = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          tex3, 
                          material, 
                          ReTexture::ReTextureDataType::color,
                          _model,
                          model->getTexture()
                        )
                  );
  }
  else {
    tex3Adtp->setMaterial(material);
    tex3Adtp->setMasterTexture(model->getTexture());
  }
  if (tex4Adtp.isNull()) {
    tex4Adtp = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          tex4, 
                          material, 
                          ReTexture::ReTextureDataType::color,
                          _model,
                          model->getTexture()
                        )
                  );
  }
  else {
    tex4Adtp->setMaterial(material);
    tex4Adtp->setMasterTexture(model->getTexture());
  }
  if (amountTextureAdp.isNull()) {
    amountTextureAdp = ReTextureEditorAdapterPtr( 
                         new ReTextureEditorAdapter(
                               this, 
                               bandMap, 
                               material, 
                               ReTexture::ReTextureDataType::numeric,
                               _model,
                               model->getTexture()
                             )
                       );
  }
  else {
    amountTextureAdp->setMaterial(material);
    amountTextureAdp->setMasterTexture(model->getTexture());
  }
  connect(offset1,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  connect(offset2,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  connect(offset3,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  connect(offset4,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  connect(amount,    SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
};


void ReBandTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  tex1->setTexture(model->getNamedValue("tex1").value<ReTexturePtr>(), material);
  tex2->setTexture(model->getNamedValue("tex2").value<ReTexturePtr>(), material);
  tex3->setTexture(model->getNamedValue("tex3").value<ReTexturePtr>(), material);
  tex4->setTexture(model->getNamedValue("tex4").value<ReTexturePtr>(), material);
  auto bandMapTex = model->getNamedValue("map").value<ReTexturePtr>();
  bandMap->setTexture(bandMapTex, material);

  offset1->setValue(model->getNamedValue("offset1").toFloat());
  offset2->setValue(model->getNamedValue("offset2").toFloat());
  offset3->setValue(model->getNamedValue("offset3").toFloat());
  offset4->setValue(model->getNamedValue("offset4").toFloat());
  amount->setValue(model->getNamedValue("amount").toDouble());
  amount->setEnabled(bandMapTex.isNull());
}

void ReBandTextureEditor::updateValue( double newVal ) {
  QString source = QObject::sender()->objectName();

  if (source.startsWith("tex")) {
    RealityBase::getUndoStack()->push(new Band_Command<double>(
                                            model, 
                                            source, 
                                            tr("Set band texture"), 
                                            newVal
                                          )
                                      );
    
  }
  else if (source.startsWith("offset")) {
    RealityBase::getUndoStack()->push(new Band_Command<double>(
                                            model, 
                                            source, 
                                            tr("Set band offset"), 
                                            newVal
                                          )
                                      );    
  }
  else if (source == "amount" ) {
    RealityBase::getUndoStack()->push(new Band_Command<double>(
                                            model, 
                                            source, 
                                            tr("Set band amount"), 
                                            newVal
                                          )
                                      );
  }
}

