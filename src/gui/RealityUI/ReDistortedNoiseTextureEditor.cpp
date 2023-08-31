/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReDistortedNoiseTextureEditor.h"

#include "RealityBase.h"
#include "ReMaterialPreview.h"
#include "ReTextureChannelDataModel.h"
#include "actions/ReTextureEditCommands.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReProceduralNoise.h"


ReDistortedNoiseTextureEditor::ReDistortedNoiseTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent),
  ReTimedEditor("tex")
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  for (int i = 0; i < (int)ReProceduralNoise::BLENDER_ORIGINAL+1; ++i) {
    noiseType->addItem(ReProceduralNoise::lutTypesToStrings[i]);
    distortionType->addItem(ReProceduralNoise::lutTypesToStrings[i]);
  }

  // Avoid getting a "modified" event every time a character is changed in the fields
  distAmount->setKeyboardTracking(false);
  noiseSize->setKeyboardTracking(false);
  brightness->setKeyboardTracking(false);
  contrast->setKeyboardTracking(false);



  previewMaker = ReMaterialPreview::getInstance();
  
  uiUpdating = false;

}

QSize ReDistortedNoiseTextureEditor::sizeHint() const {
  return QSize(273, 402);
}



void ReDistortedNoiseTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  disconnect(distortionType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateDistortionType(int)) );
  disconnect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(distAmount, SIGNAL(valueChanged(double)),        this, SLOT(updateValue(double)) );
  disconnect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(mapping3D,   SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  disconnect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));

  removeTimer();

  ReAbstractTextureEditor::setDataModel(_model, mat);
  mapping3D->setDataModel(_model, mat);
  refreshUI();

  connect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  connect(distortionType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateDistortionType(int)) );
  connect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(distAmount, SIGNAL(valueChanged(double)),        this, SLOT(updateValue(double)) );
  connect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(mapping3D,   SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  // Timer for the update of the texture preview. 
  // We don't want to call for a refresh of the texture preview for each
  // small edit of a numeric field. We start a timer and if there are no
  // other updates during a 1/2 second then the texture preview is requsted

  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(requestPreview()));

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  requestPreview();
};



void ReDistortedNoiseTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  noiseType->setCurrentIndex(model->getNamedValue("noiseBasis").toInt());
  distortionType->setCurrentIndex(model->getNamedValue("distortionType").toInt());
  distAmount->setValue(model->getNamedValue("distortionAmount").toDouble());
  noiseSize->setValue(model->getNamedValue("noiseSize").toDouble());
  brightness->setValue(model->getNamedValue("brightness").toDouble());
  contrast->setValue(model->getNamedValue("contrast").toDouble());
}



void ReDistortedNoiseTextureEditor::updateValue( double newVal ) {
  QObject* source = QObject::sender();
  if (source->objectName() == "noiseSize") {
    RealityBase::getUndoStack()->push(new DN_Command<double>(
                                            model, 
                                            "noiseSize", 
                                            tr("Set DN noise size"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "distAmount") {
    RealityBase::getUndoStack()->push(new DN_Command<double>(
                                            model, 
                                            "distortionAmount", 
                                            tr("Set DN distortion amount"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "brightness") {
    RealityBase::getUndoStack()->push(new DN_Command<double>(
                                            model, 
                                            "brightness", 
                                            tr("Set DN brightness"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "contrast") {
    RealityBase::getUndoStack()->push(new DN_Command<double>(
                                            model, 
                                            "contrast", 
                                            tr("Set DN noise size"), 
                                            newVal
                                          )
                                      );
  }
  resetTimer();  
}


void ReDistortedNoiseTextureEditor::requestPreview() {
  stopTimer();
  msgRenderingPreview->setText(tr("Rendering texture..."));
  ReTexturePtr tex = model->getTexture();
  ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(tex);
  QString texture = exporter->exportTexture(tex, "Texture", true);

  ReMaterialPreview::getInstance()->sendPreviewRequest(
    "distNoise","distNoise", "-", texture, true, true
  );    
  connect( previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
           this, SLOT(updatePreview(QString, QString,QImage*)) );
}


void ReDistortedNoiseTextureEditor::updateNoiseType( int noiseIndex ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new DN_Command<int>(
                                          model, 
                                          "noiseBasis", 
                                          tr("Set DN noise type"), 
                                          noiseIndex
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}

void ReDistortedNoiseTextureEditor::updateDistortionType( int noiseIndex ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new DN_Command<int>(
                                          model, 
                                          "distortionType", 
                                          tr("Set DN distortion type"), 
                                          noiseIndex
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}


void ReDistortedNoiseTextureEditor::updatePreview(QString matName, 
                                                  QString previewID,
                                                  QImage* preview) 
{
  if (matName != "distNoise") {
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




