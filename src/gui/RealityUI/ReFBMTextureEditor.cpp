/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReFBMTextureEditor.h"

#include <QUndoStack>

#include "RealityBase.h"
#include "ReMaterialPreview.h"
#include "ReTextureChannelDataModel.h"
#include "actions/ReTextureEditCommands.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"


ReFBMTextureEditor::ReFBMTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent),
  ReTimedEditor("tex")
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  // Avoid getting a "modified" event every time a character is changed in the fields
  octaves->setKeyboardTracking(false);
  roughness->setKeyboardTracking(false);


  previewMaker = ReMaterialPreview::getInstance();
  
  uiUpdating = false;

}

QSize ReFBMTextureEditor::sizeHint() const{
  return QSize(235, 302);
}



void ReFBMTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(octaves,   SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)) );
  disconnect(roughness, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  disconnect(wrinkled,  SIGNAL(toggled(bool)), this, SLOT(updateWrinkled(bool)) );
  disconnect(mapping3D, SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  removeTimer();

  ReAbstractTextureEditor::setDataModel(_model, mat);
  mapping3D->setDataModel(_model, mat);
  refreshUI();

  connect(octaves,   SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)) );
  connect(roughness, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );
  connect(wrinkled,  SIGNAL(toggled(bool)), this, SLOT(updateWrinkled(bool)) );
  connect(mapping3D, SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  // Timer for the update of the texture preview. 
  // We don't want to call for a refresh of the texture preview for each
  // small edit of a numeric field. We start a timer and if there are no
  // other updates during a 1/2 second then the texture preview is requested
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(requestPreview()));

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  requestPreview();
};



void ReFBMTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  octaves->setValue(model->getNamedValue("octaves").toInt());
  roughness->setValue(model->getNamedValue("roughness").toFloat());
}



void ReFBMTextureEditor::updateValue( int newVal ) {
  QObject* source = QObject::sender();
  if (source->objectName() == "octaves") {
    RealityBase::getUndoStack()->push(new BE_Command<float>(
                                            model, 
                                            "octaves", 
                                            tr("Set num layers"), 
                                            newVal
                                          )
                                      );
  }
  resetTimer();  
}

void ReFBMTextureEditor::updateValue( double newVal ) {
  RealityBase::getUndoStack()->push(new FBME_Command<double>(
                                          model, 
                                          "roughness", 
                                          tr("Set roughness"), 
                                          newVal
                                        )
                                    );
  resetTimer();
}

void ReFBMTextureEditor::updateWrinkled( bool newVal ) {
  RealityBase::getUndoStack()->push(new FBME_Command<bool>(
                                          model, 
                                          "wrinkled", 
                                          tr("Set to wrinkled"), 
                                          newVal
                                        )
                                    );
  resetTimer();
}

void ReFBMTextureEditor::requestPreview() {
  stopTimer();
  msgRenderingPreview->setText(tr("Rendering texture..."));
  ReTexturePtr tex = model->getTexture();
  ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(tex);
  QString texture = exporter->exportTexture(tex, "Texture", true);

  connect( previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
           this, SLOT(updatePreview(QString, QString,QImage*)) );
  ReMaterialPreview::getInstance()->sendPreviewRequest(
    "fbm","fbm", "-", texture, true, true
  );    
}

void ReFBMTextureEditor::updatePreview(QString matName, QString previewID, QImage* preview ) 
{
  if (matName != "fbm") {
    return;
  }
  noiseResult->setPixmap( QPixmap::fromImage( *preview ) );
  // We are responsible for the disposal of this resources
  delete preview;
  msgRenderingPreview->setText("");
  // We disconnect to avoid receiving messages for other textures or materials
  // when this texture editor is not visible.
  disconnect( previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
              this, SLOT(updatePreview(QString, QString,QImage*)) );
}




