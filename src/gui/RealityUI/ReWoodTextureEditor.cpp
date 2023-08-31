/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/


#include "RealityUI/ReWoodTextureEditor.h"

#include <QUndoStack>

#include "RealityBase.h"
#include "ReMaterialPreview.h"
#include "ReTextureChannelDataModel.h"
#include "actions/ReTextureEditCommands.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReProceduralNoise.h"
#include "textures/ReWood.h"


ReWoodTextureEditor::ReWoodTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent),
  ReTimedEditor("tex")
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  for (int i = 0; i < (int)ReProceduralNoise::BLENDER_ORIGINAL+1; ++i) {
    noiseType->addItem(ReProceduralNoise::lutTypesToStrings[i]);
  }
  for (int i = 0; i < (int)ReWood::RING_NOISE+1; ++i) {
    woodPattern->addItem(ReWood::lutPatternsToStrings[i]);
  }

  // Avoid getting a "modified" event every time a character is changed in the fields
  noiseSize->setKeyboardTracking(false);
  turbulence->setKeyboardTracking(false);
  brightness->setKeyboardTracking(false);
  contrast->setKeyboardTracking(false);


  previewMaker = ReMaterialPreview::getInstance();  


  uiUpdating = false;
}

QSize ReWoodTextureEditor::sizeHint() const {
  return QSize(312, 480);
}



void ReWoodTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  disconnect(woodPattern,SIGNAL(currentIndexChanged(int)), this, SLOT(updateWoodPattern(int)) );
  disconnect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(turbulence, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(mapping3D,  SIGNAL(mappingChanged()),         this, SLOT(requestPreview()) );

  // Vein wave selection
  disconnect(sinWave,      SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );
  disconnect(sawtoothWave, SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );
  disconnect(triangleWave, SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );

  disconnect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));

  removeTimer();

  ReAbstractTextureEditor::setDataModel(_model, mat);
  mapping3D->setDataModel(_model, mat);
  refreshUI();

  connect(sinWave,      SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );
  connect(sawtoothWave, SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );
  connect(triangleWave, SIGNAL(toggled(bool)), this, SLOT(updateVeinWave(bool)) );

  connect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  connect(woodPattern,SIGNAL(currentIndexChanged(int)), this, SLOT(updateWoodPattern(int)) );
  connect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(turbulence, SIGNAL(valueChanged(double)),        this, SLOT(updateValue(double)) );
  connect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(mapping3D,  SIGNAL(mappingChanged()),         this, SLOT(requestPreview()) );

  connect(softNoise, SIGNAL(toggled(bool)), this, SLOT(updateNoiseHardness(bool)) );

  // Timer for the update of the texture preview. 
  // We don't want to call for a refresh of the texture preview for each
  // small edit of a numeric field. We start a timer and if there are no
  // other updates during a 1/2 second then the texture preview is requsted
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(requestPreview()));

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  requestPreview();
};



void ReWoodTextureEditor::refreshUI() {

  if (uiUpdating) {
    return;
  }

  switch( model->getNamedValue("veinWave").toInt() ) {
    case ReWood::SIN:
      sinWave->setChecked(true);
      break;
    case ReWood::SAWTOOTH:
      sawtoothWave->setChecked(true);
      break;
    case ReWood::TRIANGLE:
      triangleWave->setChecked(true);
      break;
  }  
  noiseType->setCurrentIndex(model->getNamedValue("noiseBasis").toInt());  
  int woodPatternInt = model->getNamedValue("woodPattern").toInt();
  woodPattern->setCurrentIndex(woodPatternInt);
  configureNoiseWidgets( woodPatternInt );

  noiseSize->setValue(model->getNamedValue("noiseSize").toDouble());
  turbulence->setValue(model->getNamedValue("turbulence").toDouble());
  brightness->setValue(model->getNamedValue("brightness").toDouble());
  contrast->setValue(model->getNamedValue("contrast").toDouble());

}



void ReWoodTextureEditor::updateValue( double newVal ) {
  QObject* source = QObject::sender();
  if (source->objectName() == "noiseSize") {
    RealityBase::getUndoStack()->push(new WE_Command<double>(
                                            model, 
                                            "noiseSize", 
                                            tr("Set Wood noise size"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "turbulence") {
    RealityBase::getUndoStack()->push(new WE_Command<double>(
                                            model, 
                                            "turbulence", 
                                            tr("Set Wood distortion amount"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "brightness") {
    RealityBase::getUndoStack()->push(new WE_Command<double>(
                                            model, 
                                            "brightness", 
                                            tr("Set Wood brightness"), 
                                            newVal
                                          )
                                      );
  }
  else if (source->objectName() == "contrast") {
    RealityBase::getUndoStack()->push(new WE_Command<double>(
                                            model, 
                                            "contrast", 
                                            tr("Set Wood noise size"), 
                                            newVal
                                          )
                                      );
  }
  resetTimer();  
}

void ReWoodTextureEditor::updateVeinWave( bool onOff ) {
  if (uiUpdating) {
    return;
  }
  if ( sinWave->isChecked() ) {
    model->setNamedValue( "veinWave", ReWood::SIN );
  }
  else if ( sawtoothWave->isChecked() ) {
    model->setNamedValue( "veinWave", ReWood::SAWTOOTH );
  }
  else if ( triangleWave->isChecked() )  {
    model->setNamedValue( "veinWave", ReWood::TRIANGLE );
  }
  resetTimer();
}

void ReWoodTextureEditor::updateNoiseHardness( bool onOff ) {
  model->setNamedValue("softNoise", onOff);
  resetTimer();
}

#define WT_PREVIEW_NAME "wood"

void ReWoodTextureEditor::requestPreview() {
  stopTimer();
  msgRenderingPreview->setText(tr("Rendering texture..."));
  ReTexturePtr tex = model->getTexture();
  ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(tex);
  QString texture = exporter->exportTexture(tex, "Texture", true);

  ReMaterialPreview::getInstance()->sendPreviewRequest(
    WT_PREVIEW_NAME, WT_PREVIEW_NAME, "-", texture, true, true
  );
  connect( previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
           this, SLOT(updatePreview(QString, QString,QImage*)) );
}


void ReWoodTextureEditor::updateNoiseType( int noiseIndex ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new WE_Command<int>(
                                          model, 
                                          "noiseBasis", 
                                          tr("Set Wood noise type"), 
                                          noiseIndex
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}

void ReWoodTextureEditor::configureNoiseWidgets( int newVal ) {
  ReWood::WoodPattern pattern = static_cast<ReWood::WoodPattern>(newVal);
  bool enableWidgets = true;
  if ( pattern == ReWood::BANDS || pattern == ReWood::RINGS ) {
    enableWidgets = false;
  }
  hardNoise->setEnabled(enableWidgets);
  softNoise->setEnabled(enableWidgets);
  noiseType->setEnabled(enableWidgets);
  noiseSize->setEnabled(enableWidgets);
  turbulence->setEnabled(enableWidgets);
}

void ReWoodTextureEditor::updateWoodPattern( int newVal ) {
  uiUpdating = true;
  configureNoiseWidgets(newVal);

  RealityBase::getUndoStack()->push(new DN_Command<int>(
                                          model, 
                                          "woodPattern", 
                                          tr("Set Wood pattern"), 
                                          newVal
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}


void ReWoodTextureEditor::updatePreview(QString matName, QString previewID,QImage* preview) 
{
  if (matName != WT_PREVIEW_NAME) {
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



