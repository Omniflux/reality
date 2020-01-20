/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include <QObject>
#include <QMessageBox>

#include "ReColorMathTextureEditor.h"
#include "textures/ReColorMath.h"
#include "ReGeometryObject.h"

using namespace Reality;

ReColorMathTextureEditor::ReColorMathTextureEditor(QWidget* parent) : 
  ReAbstractTextureEditor(parent) 
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  // Set the allowed data types for the textures
  texture1->setAllowedDataTypes(RE_TEX_DATATYPE_COLOR);
  texture2->setAllowedDataTypes(RE_TEX_DATATYPE_COLOR);
  texture2->setDeleteEnabled(true);

  connect(color1, SIGNAL(colorChanged()), this, SLOT(updateColor1()));
  connect(color2, SIGNAL(colorChanged()), this, SLOT(updateColor2()));
  connect(function, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFunction(int)));
}

QSize ReColorMathTextureEditor::sizeHint() const {
  return QSize(300,534);
}

void ReColorMathTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, 
                                             ReMaterial* mat ) 
{
  dontSignal = true;
  ReAbstractTextureEditor::setDataModel(_model, mat);

  if (Tex1Adapter.isNull()) {
    Tex1Adapter = ReTextureEditorAdapterPtr( 
                    new ReTextureEditorAdapter(
                          this, 
                          texture1, 
                          material, 
                          ReTexture::ReTextureDataType::color,
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
                          ReTexture::ReTextureDataType::color,
                          _model,
                          model->getTexture()
                        )
                  );
  }
  else {
    Tex2Adapter->setMaterial(material);    
    Tex2Adapter->setMasterTexture(model->getTexture());
  }
  color1->setColor(model->getNamedValue("color1").value<QColor>());
  color2->setColor(model->getNamedValue("color2").value<QColor>());
  texture1->setTexture(model->getNamedValue("texture1").value<ReTexturePtr>(), material);
  texture2->setTexture(model->getNamedValue("texture2").value<ReTexturePtr>(), material);
  function->setCurrentIndex(model->getNamedValue("function").toInt());
  dontSignal = false;
}

void ReColorMathTextureEditor::updateColor1() {
  if (dontSignal) {
    return;
  }
  model->setNamedValue("color1", color1->getColor());
}

void ReColorMathTextureEditor::updateColor2() {
  if (dontSignal) {
    return;
  }
  model->setNamedValue("color2", color2->getColor());
}

void ReColorMathTextureEditor::updateFunction(int newVal) {
  ReColorMath::functions func = static_cast<ReColorMath::functions>(newVal);
  if (!dontSignal) {
    model->setNamedValue("function", func);
  }
  if (func == ReColorMath::none) {
    frTexture2->hide();
  }
  else {
    frTexture2->show();
  }
}

