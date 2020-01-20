/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReBricks.h"
#include "ReBricksTextureEditor.h"
#include "textures/ReProceduralNoise.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"

ReBricksTextureEditor::ReBricksTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent),
  ReTimedEditor("tex")
{

  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  for (int i = 0; i < (int)Bricks::CHAINLINK+1; ++i) {
    brickType->addItem(Bricks::lutTypesToStrings[i]);
  }

  // Avoid getting a "modified" event every time a character is changed in the fields
  brickWidth->setKeyboardTracking(false);
  brickHeight->setKeyboardTracking(false);
  brickDepth->setKeyboardTracking(false);
  brickBevel->setKeyboardTracking(false);
  mortarSize->setKeyboardTracking(false);
  brickOffset->setKeyboardTracking(false);
  // scale->setKeyboardTracking(false);

  previewMaker = ReMaterialPreview::getInstance();
  
  uiUpdating = false;
}

void ReBricksTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(brickType,   SIGNAL(currentIndexChanged(int)), this, SLOT(updateBrickType(int)) );
  disconnect(brickWidth,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(brickHeight, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(brickDepth,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(brickBevel,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(brickOffset, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(mortarSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  disconnect(mapping3D,   SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  disconnect(brickTexture, SIGNAL(editTexture(ReTexturePtr, QString&)), 
             this,         SIGNAL(editTexture(ReTexturePtr, QString&)));
  disconnect(mortarTexture, SIGNAL(editTexture(ReTexturePtr, QString&)), 
             this,          SIGNAL(editTexture(ReTexturePtr, QString&)));
  disconnect(brickModulation, SIGNAL(editTexture(ReTexturePtr, QString&)), 
             this,            SIGNAL(editTexture(ReTexturePtr, QString&)));

  removeTimer();

  ReAbstractTextureEditor::setDataModel(_model, mat);
  mapping3D->setDataModel(_model, mat);
  refreshUI();

  connect(brickType,   SIGNAL(currentIndexChanged(int)), this, SLOT(updateBrickType(int)) );
  connect(brickWidth,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(brickHeight, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(brickDepth,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(brickBevel,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(brickOffset, SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(mortarSize,  SIGNAL(valueChanged(double)),     this, SLOT(updateValue(double)) );
  connect(mapping3D,   SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  connect(brickTexture, SIGNAL(editTexture(ReTexturePtr, QString&)), 
          this,         SIGNAL(editTexture(ReTexturePtr, QString&)));
  connect(mortarTexture, SIGNAL(editTexture(ReTexturePtr, QString&)), 
          this,          SIGNAL(editTexture(ReTexturePtr, QString&)));
  connect(brickModulation, SIGNAL(editTexture(ReTexturePtr, QString&)), 
          this,            SIGNAL(editTexture(ReTexturePtr, QString&)));

  // Timer for the update of the texture preview. 
  // We don't want to call for a refresh of the texture preview for each
  // small edit of a numeric field. We start a timer and if there are no
  // other updates during a 1/2 second then the texture preview is requsted

  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(requestPreview()));

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  requestPreview();
};

void ReBricksTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  brickType->setCurrentIndex(model->getNamedValue("brickType").toInt());
  brickWidth->setValue(model->getNamedValue("width").toFloat());
  brickHeight->setValue(model->getNamedValue("height").toFloat());
  brickDepth->setValue(model->getNamedValue("depth").toFloat());
  brickBevel->setValue(model->getNamedValue("bevel").toFloat());
  brickOffset->setValue(model->getNamedValue("offset").toFloat());
  mortarSize->setValue(model->getNamedValue("mortarSize").toFloat());
  // scale->setValue(model->getNamedValue("scale").toFloat()*100);

  brickTexture->setTexture( model->getNamedValue("brickTexture").value<ReTexturePtr>(), 
                            material );
  mortarTexture->setTexture( model->getNamedValue("mortarTexture").value<ReTexturePtr>(), 
                             material );
  brickModulation->setTexture( model->getNamedValue("modTexture").value<ReTexturePtr>(), 
                               material );

}

void ReBricksTextureEditor::updateValue( double newVal ) {
  QString source = QObject::sender()->objectName();
  if (source == "brickWidth") {
    RealityBase::getUndoStack()->push(new BE_Command<double>(
                                            model, 
                                            "width", 
                                            tr("Set brick width"), 
                                            newVal
                                          )
                                      );
  }
  else if (source == "brickHeight") {
    RealityBase::getUndoStack()->push(new BE_Command<double>(
                                            model, 
                                            "height", 
                                            tr("Set brick height"), 
                                            newVal
                                          )
                                      );
  }
  else if (source == "brickDepth") {
    RealityBase::getUndoStack()->push(new BE_Command<double>(
                                            model, 
                                            "depth", 
                                            tr("Set brick depth"), 
                                            newVal
                                          )
                                      );
  }
  else if (source == "brickBevel") {
    RealityBase::getUndoStack()->push(new BE_Command<double>(
                                            model, 
                                            "bevel", 
                                            tr("Set brick bevel"), 
                                            newVal
                                          )
                                      );
  }
  else if (source == "brickHeight") {
    RealityBase::getUndoStack()->push(new BE_Command<double>(
                                            model, 
                                            "height", 
                                            tr("Set brick height"), 
                                            newVal
                                          )
                                      );
  }
  else if (source == "brickOffset") {
    RealityBase::getUndoStack()->push(new BE_Command<double>(
                                            model, 
                                            "offset", 
                                            tr("Set brick offset"), 
                                            newVal
                                          )
                                      );
  }
  else if (source == "mortarSize") {
    RealityBase::getUndoStack()->push(new BE_Command<double>(
                                            model, 
                                            "mortarSize", 
                                            tr("Set mortar size"), 
                                            newVal
                                          )
                                      );
  }
  resetTimer();  
}


void ReBricksTextureEditor::requestPreview() {
  stopTimer();
  msgRenderingPreview->setText(tr("Rendering texture..."));
  ReTexturePtr tex = model->getTexture();
  ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(tex);
  QString texture = exporter->exportTexture(tex, "Texture", true);

  ReMaterialPreview::getInstance()->sendPreviewRequest(
    "bricks", "bricks", "-", texture, true, true
  );
  connect( previewMaker, SIGNAL(previewReady(QString,QString,QImage*)), 
           this, SLOT(updatePreview(QString,QString,QImage*)) );
}


void ReBricksTextureEditor::updateBrickType( int brickIndex ) {
  uiUpdating = true;
  RealityBase::getUndoStack()->push(new BE_Command<int>(
                                          model, 
                                          "brickType", 
                                          tr("Set brick type"), 
                                          brickIndex
                                        )
                                    );
  uiUpdating = false;
  resetTimer();
}


void ReBricksTextureEditor::updatePreview( QString matName, 
                                           QString previewID,
                                           QImage* preview ) 
{
  if (matName != "bricks") {
    return;
  }
  noiseResult->setPixmap( QPixmap::fromImage( *preview ) );
  // We are responsible for the disposal of this resources
  delete preview;
  msgRenderingPreview->setText("");
  // We disconnect to avoid receiving messages for other textures or materials
  // when this texture editor is not visible.
  disconnect( previewMaker, SIGNAL(previewReady(QString,QString,QImage*)), 
              this, SLOT(updatePreview(QString,QString,QImage*)) );
}
