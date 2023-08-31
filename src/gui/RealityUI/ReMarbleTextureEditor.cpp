/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReMarbleTextureEditor.h"

#include <QUndoStack>

#include "RealityBase.h"
#include "ReMaterial.h"
#include "ReMaterialPreview.h"
#include "ReTextureChannelDataModel.h"
#include "actions/ReTextureEditCommands.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReMarble.h"
#include "textures/ReProceduralNoise.h"


ReMarbleTextureEditor::ReMarbleTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent),
  ReTimedEditor("tex")
{

  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  for (int i = 0; i < (int)ReProceduralNoise::BLENDER_ORIGINAL+1; ++i) {
    noiseType->addItem(ReProceduralNoise::lutTypesToStrings[i]);
  }

  // Avoid getting a "modified" event every time a character is changed in the fields
  noiseSize->setKeyboardTracking(false);
  turbulence->setKeyboardTracking(false);
  noiseDepth->setKeyboardTracking(false);
  brightness->setKeyboardTracking(false);
  contrast->setKeyboardTracking(false);


  previewMaker = ReMaterialPreview::getInstance();  


  uiUpdating = false;
}



void ReMarbleTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  disconnect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(noiseDepth, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(turbulence, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(mapping3D,   SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  // Vein quality selection
  disconnect(softVeins,    SIGNAL(toggled(bool)), this, SLOT(updateVeinQuality(bool)) );
  disconnect(sharpVeins,   SIGNAL(toggled(bool)), this, SLOT(updateVeinQuality(bool)) );
  disconnect(sharperVeins, SIGNAL(toggled(bool)), this, SLOT(updateVeinQuality(bool)) );

  // Vein wave selection
  disconnect(sinWave,      SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );
  disconnect(sawtoothWave, SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );
  disconnect(triangleWave, SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );

  disconnect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));

  removeTimer();

  ReAbstractTextureEditor::setDataModel(_model, mat);
  mapping3D->setDataModel(_model, mat);
  refreshUI();

  connect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  connect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(noiseDepth, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(turbulence, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(mapping3D,  SIGNAL(mappingChanged()),         this, SLOT(requestPreview()) );

  connect(softVeins,    SIGNAL(toggled(bool)), this, SLOT(updateVeinQuality(bool)) );
  connect(sharpVeins,   SIGNAL(toggled(bool)), this, SLOT(updateVeinQuality(bool)) );
  connect(sharperVeins, SIGNAL(toggled(bool)), this, SLOT(updateVeinQuality(bool)) );

  connect(softNoise, SIGNAL(toggled(bool)), this, SLOT(updateNoiseHardness(bool)) );

  // Timer for the update of the texture preview. 
  // We don't want to call for a refresh of the texture preview for each
  // small edit of a numeric field. We start a timer and if there are no
  // other updates during a 1/2 second then the texture preview is requested

  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(requestPreview()));

  connect(model.data(), SIGNAL(modelChanged()), SLOT(refreshUI()));
  requestPreview();
};



void ReMarbleTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }

  switch( model->getNamedValue("veinQuality").toInt() ) {
    case ReMarble::SOFT:
      softVeins->setChecked(true);
      break;
    case ReMarble::SHARP:
      sharpVeins->setChecked(true);
      break;
    case ReMarble::SHARPER:
      sharperVeins->setChecked(true);
      break;
  }
  switch( model->getNamedValue("veinWave").toInt() ) {
    case ReMarble::SIN:
      sinWave->setChecked(true);
      break;
    case ReMarble::SAWTOOTH:
      sawtoothWave->setChecked(true);
      break;
    case ReMarble::TRIANGLE:
      triangleWave->setChecked(true);
      break;
  }
  noiseType->setCurrentIndex(model->getNamedValue("noiseType").toInt());
  noiseSize->setValue(model->getNamedValue("noiseSize").toDouble());
  noiseDepth->setValue(model->getNamedValue("depth").toDouble());
  turbulence->setValue(model->getNamedValue("turbulence").toDouble());
  brightness->setValue(model->getNamedValue("brightness").toDouble());
  contrast->setValue(model->getNamedValue("contrast").toDouble());
}



void ReMarbleTextureEditor::updateValue( double newVal ) {
  QObject* source = QObject::sender();
  if (source->objectName() == "noiseSize") {
    RealityBase::getUndoStack()->push(new MA_Command<double>(
                                            model, 
                                            "noiseSize", 
                                            tr("Set DN noise size"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "depth") {
    RealityBase::getUndoStack()->push(new MA_Command<double>(
                                            model, 
                                            "depth", 
                                            tr("Set DN distortion amount"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "turbulence") {
    RealityBase::getUndoStack()->push(new MA_Command<double>(
                                            model, 
                                            "turbulence", 
                                            tr("Set DN distortion amount"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "brightness") {
    RealityBase::getUndoStack()->push(new MA_Command<double>(
                                            model, 
                                            "brightness", 
                                            tr("Set DN brightness"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "contrast") {
    RealityBase::getUndoStack()->push(new MA_Command<double>(
                                            model, 
                                            "contrast", 
                                            tr("Set DN noise size"), 
                                            newVal
                                          )
                                      );
  }
  resetTimer();  
}

void ReMarbleTextureEditor::updateVeinQuality( bool onOff ) {
  if (uiUpdating) {
    return;
  }
  if ( softVeins->isChecked() ) {
    model->setNamedValue( "veinQuality", ReMarble::SOFT );
  }
  else if ( sharpVeins->isChecked() ) {
    model->setNamedValue( "veinQuality", ReMarble::SHARP );
  }
  else if ( sharperVeins->isChecked() )  {
    model->setNamedValue( "veinQuality", ReMarble::SHARPER );
  }
  resetTimer();
}

void ReMarbleTextureEditor::updateVeinWave( bool onOff ) {
  if (uiUpdating) {
    return;
  }
  if ( sinWave->isChecked() ) {
    model->setNamedValue( "veinQuality", ReMarble::SIN );
  }
  else if ( sawtoothWave->isChecked() ) {
    model->setNamedValue( "veinQuality", ReMarble::SAWTOOTH );
  }
  else if ( triangleWave->isChecked() )  {
    model->setNamedValue( "veinQuality", ReMarble::TRIANGLE );
  }
}

void ReMarbleTextureEditor::updateNoiseHardness( bool onOff ) {
  model->setNamedValue("softNoise", onOff);
}

void ReMarbleTextureEditor::requestPreview() {
  stopTimer();
  msgRenderingPreview->setText(tr("Rendering texture..."));
  ReTexturePtr tex = model->getTexture();
  ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(tex);
  QString texture = exporter->exportTexture(tex, "Texture", true);

  ReMaterialPreview::getInstance()->sendPreviewRequest(
    "marble","marble", "-", texture, true, true
  );    
  connect( previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
           this, SLOT(updatePreview(QString, QString,QImage*)) );
}


void ReMarbleTextureEditor::updateNoiseType( int noiseIndex ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new DN_Command<int>(
                                          model, 
                                          "noiseType", 
                                          tr("Set DN noise type"), 
                                          noiseIndex
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}


void ReMarbleTextureEditor::updatePreview(QString matName, QString previewID,QImage* preview) 
{
  if (matName != "marble") {
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




