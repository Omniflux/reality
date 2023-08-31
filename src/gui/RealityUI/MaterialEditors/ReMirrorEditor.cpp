/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReMirrorEditor.h"


using namespace Reality;

ReMirrorEditor::ReMirrorEditor( QWidget* parent ) : 
  ReMaterialEditor(parent) 
{
  setupUi(this);
  currentMaterial = NULL;
  inSetup = false;
  // setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

  filmThickness->setKeyboardTracking(false);
  filmIOR->setKeyboardTracking(false);
  // connect(Kr,   SIGNAL(editTexture(ReTexturePtr, QString&)), 
  //         this, SIGNAL(editTexture(ReTexturePtr, QString&)));

  KrAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      Kr, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );

  connect(filmThickness, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(filmIOR, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
}

QSize ReMirrorEditor::sizeHint() const {
  return QSize(474, 145);    
}

void ReMirrorEditor::setData( ReMaterial* newMat, 
                              ReTextureChannelDataModelPtr tcm ) 
{
  inSetup = true;

  currentMaterial = newMat;
  textureChannelModel = tcm;

  KrAdapter->setMaterial(newMat);
  KrAdapter->setTextureChannelModel(tcm);

  Kr->setTexture(currentMaterial->getNamedValue("Kr").value<ReTexturePtr>(), currentMaterial);
  filmThickness->setValue(currentMaterial->getNamedValue("filmThickness").toFloat());
  filmIOR->setValue(currentMaterial->getNamedValue("filmIOR").toFloat());
  inSetup = false;
}

void ReMirrorEditor::updateValue( double newVal ) {
  if (inSetup) {
    return;
  }
  QString source = QObject::sender()->objectName();
  if (source == "filmThickness") {
    currentMaterial->setNamedValue("filmThickness", newVal);
  }
  else if (source == "filmIOR") {
    currentMaterial->setNamedValue("filmIOR", newVal);
  }
  currentMaterial->setEdited();
  signalUpdate(source, newVal);
} 

