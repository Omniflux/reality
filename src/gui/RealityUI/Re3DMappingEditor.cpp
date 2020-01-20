/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "Re3DMappingEditor.h"
#include "RealityBase.h"

Re3DMappingEditor::Re3DMappingEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent) 
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

  // Avoid getting a "modified" event every time a character is changed in the fields
  scale->setKeyboardTracking(false);
  uiUpdating = false;
}

QSize Re3DMappingEditor::sizeHint() const { 
  return QSize(250, 42);
}

void Re3DMappingEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(scale,     SIGNAL(valueChanged(double)), this, SLOT(updateScale(double)) );
  disconnect(mapping,   SIGNAL(activated(const QString&)), this, SLOT(updateMapping(const QString&)) );

  ReAbstractTextureEditor::setDataModel(_model, mat);
  refreshUI();

  connect(scale,     SIGNAL(valueChanged(double)), this, SLOT(updateScale(double)) );
  connect(mapping,   SIGNAL(activated(const QString&)), this, SLOT(updateMapping(const QString&)) );

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
};

void Re3DMappingEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  scale->setValue(model->getNamedValue("scale").toFloat()*100.0f);
  ReTexture3D::Mapping3D mapType = static_cast<ReTexture3D::Mapping3D>(
    model->getNamedValue("mapping").toInt()
  );
  QString mapStr;
  switch( mapType ) {
    case ReTexture3D::UV:
      mapStr = "uv";
      break;
    case ReTexture3D::LOCAL:
      mapStr = "local";
      break;
    case ReTexture3D::GLOBAL:
      mapStr = "global";
      break;
  }
  int pos = mapping->findText( mapStr, Qt::MatchFixedString );
  if ( pos != -1 ) {
    mapping->setCurrentIndex( pos );
  }
}

void Re3DMappingEditor::updateScale( double newVal ) {
  uiUpdating = true;
  float newValF = newVal / 100.0f;
  RealityBase::getUndoStack()->push(new M3D_Command<float>(
                                          model, 
                                          "scale", 
                                          tr("Set scale factor"), 
                                          newValF
                                        )
                                    );
  emit mappingChanged();
  uiUpdating = false;
}

void Re3DMappingEditor::updateMapping( const QString& newVal ) {
  uiUpdating = true;
  ReTexture3D::Mapping3D mapType;
  QString mapChoice = newVal.toLower();
  if ( mapChoice == "uv" ) {
    mapType = ReTexture3D::UV;
  }
  else if ( mapChoice == "global" ) {
    mapType = ReTexture3D::GLOBAL;
  }
  else if ( mapChoice == "local" ) {
    mapType = ReTexture3D::LOCAL;
  }
  RealityBase::getUndoStack()->push(new M3D_Command<int>(
                                          model, 
                                          "mapping", 
                                          tr("Set texture mapping"), 
                                          mapType
                                        )
                                   );
  emit mappingChanged();
  uiUpdating = false;
}
