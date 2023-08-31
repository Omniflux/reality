/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReCloudsTextureEditor.h"

#include <QUndoStack>

#include "RealityBase.h"
#include "ReMaterialPreview.h"
#include "ReTextureChannelDataModel.h"
#include "actions/ReTextureEditCommands.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReProceduralNoise.h"


ReCloudsTextureEditor::ReCloudsTextureEditor( QWidget* parent ) : 
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
  noiseDepth->setKeyboardTracking(false);
  brightness->setKeyboardTracking(false);
  contrast->setKeyboardTracking(false);

  previewMaker = ReMaterialPreview::getInstance();

  msgRenderingPreview->hide();

  uiUpdating = false;
}

QSize ReCloudsTextureEditor::sizeHint() const {
  return QSize(280, 554);
}

void ReCloudsTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  disconnect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateNoiseSize(double)) );
  disconnect(noiseDepth, SIGNAL(valueChanged(int)),        this, SLOT(updateNoiseDepth(int)) );
  disconnect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateBrightness(double)) );
  disconnect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateContrast(double)) );
  disconnect(hardNoise,  SIGNAL(toggled(bool)),            this, SLOT(setHardSoftNoise(bool)) );
  disconnect(mapping3D,  SIGNAL(mappingChanged()),         this, SLOT(requestPreview()) );

  removeTimer();

  ReAbstractTextureEditor::setDataModel(_model, mat);
  mapping3D->setDataModel(_model, mat);
  refreshUI();
  setNoisePattern(model->getNamedValue("noiseType").toInt());

  connect(noiseType,  SIGNAL(currentIndexChanged(int)), this, SLOT(updateNoiseType(int)) );
  connect(noiseSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateNoiseSize(double)) );
  connect(noiseDepth, SIGNAL(valueChanged(int)),        this, SLOT(updateNoiseDepth(int)) );
  connect(brightness, SIGNAL(valueChanged(double)),     this, SLOT(updateBrightness(double)) );
  connect(contrast,   SIGNAL(valueChanged(double)),     this, SLOT(updateContrast(double)) );
  connect(hardNoise,  SIGNAL(toggled(bool)),            this, SLOT(setHardSoftNoise(bool)) );
  connect(mapping3D,  SIGNAL(mappingChanged()),         this, SLOT(requestPreview()) );

  // Timer for the update of the texture preview. 
  // We don't want to call for a refresh of the texture preview for each
  // small edit of a numeric field. We start a timer and if there are no
  // other updates during a 1/2 second then the texture preview is requested
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(requestPreview()));

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  requestPreview();
};

void ReCloudsTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  noiseType->setCurrentIndex(model->getNamedValue("noiseType").toInt());
  noiseSize->setValue(model->getNamedValue("noiseSize").toDouble());
  brightness->setValue(model->getNamedValue("brightness").toDouble());
  contrast->setValue(model->getNamedValue("contrast").toDouble());
  noiseDepth->setValue(model->getNamedValue("noiseDepth").toDouble());
  if (model->getNamedValue("usesHardNoise").toBool()) {
    hardNoise->setChecked(true);
  }
  else {
    softNoise->setChecked(true);
  }
}


void ReCloudsTextureEditor::setNoisePattern( int noiseIndex ) {
  QPixmap pattern;
  switch(noiseIndex) {
    case ReProceduralNoise::CELL_NOISE:
      pattern.load(":/images/noise_cell.png");
      break;
    case ReProceduralNoise::VORONOI_CRACKLE:
      pattern.load(":/images/noise_voronoi_crackle.png");
      break;
    case ReProceduralNoise::VORONOI_F2_F1:
      pattern.load(":/images/noise_voronoi_f2-f1.png");
      break;
    case ReProceduralNoise::VORONOI_F4:
      pattern.load(":/images/noise_voronoi_f4.png");
      break;
    case ReProceduralNoise::VORONOI_F3:
      pattern.load(":/images/noise_voronoi_f3.png");
      break;
    case ReProceduralNoise::VORONOI_F2:
      pattern.load(":/images/noise_voronoi_f2.png");
      break;
    case ReProceduralNoise::VORONOI_F1:
      pattern.load(":/images/noise_voronoi_f1.png");
      break;
    case ReProceduralNoise::IMPROVED_PERLIN:
      pattern.load(":/images/noise_improved_perlin.png");
      break;
    case ReProceduralNoise::ORIGINAL_PERLIN:
      pattern.load(":/images/noise_original_perlin.png");
      break;
    case ReProceduralNoise::BLENDER_ORIGINAL:
      pattern.load(":/images/noise_blender_original.png");
      break;
  }
  noisePattern->setPixmap(pattern);    
}

void ReCloudsTextureEditor::requestPreview() {
  stopTimer();
  msgRenderingPreview->show();
  ReTexturePtr tex = model->getTexture();
  ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(tex);
  QString texture = exporter->exportTexture(tex, "Texture", true);

  ReMaterialPreview::getInstance()->sendPreviewRequest(
    "clouds","clouds", "-", texture, true, true
  );    
  connect(previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
          this, SLOT(updatePreview(QString, QString,QImage*)));
}


void ReCloudsTextureEditor::updateNoiseType( int noiseIndex ) {

  setNoisePattern(noiseIndex);

  uiUpdating = true;
  RealityBase::getUndoStack()->push(new CE_Command<int>(
                                          model, 
                                          "noiseType", 
                                          tr("Set noise type"), 
                                          noiseIndex
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}


void ReCloudsTextureEditor::updatePreview(QString matName, QString previewID, QImage* preview ) {
  if (matName != "clouds") {
    return;
  }
  noiseResult->setPixmap(QPixmap::fromImage(*preview));
  // We are responsible for the disposal of this resources
  delete preview;
  msgRenderingPreview->hide();
  // We disconnect to avoid receiving messages for other textures or materials
  // when this texture editor is not visible.
  disconnect(previewMaker, SIGNAL(previewReady(QString, QString,QImage*)), 
             this, SLOT(updatePreview(QString, QString,QImage*)));
}

  
void ReCloudsTextureEditor::updateNoiseSize( double newVal ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new CE_Command<double>(
                                          model, 
                                          "noiseSize", 
                                          tr("Set noise size"), 
                                          newVal
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}

void ReCloudsTextureEditor::updateNoiseDepth( int newVal ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new CE_Command<int>(
                                          model, 
                                          "noiseDepth", 
                                          tr("Set noise depth"), 
                                          newVal
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}

void ReCloudsTextureEditor::updateBrightness( double newVal ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new CE_Command<double>(
                                          model, 
                                          "brightness", 
                                          tr("Set brightness"), 
                                          newVal
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}

void ReCloudsTextureEditor::updateContrast( double newVal ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new CE_Command<double>(
                                          model, 
                                          "contrast", 
                                          tr("Set contrast"), 
                                          newVal
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}

void ReCloudsTextureEditor::setHardSoftNoise( bool ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new CE_Command<bool>(
                                          model, 
                                          "usesHardNoise", 
                                          tr("Set noise quality"), 
                                          hardNoise->isChecked()
                                        )
                                    );
  uiUpdating = false;
  requestPreview();
}
