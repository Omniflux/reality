/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReCheckersTextureEditor.h"
#include "RealityBase.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"

ReCheckersTextureEditor::ReCheckersTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent),
  ReTimedEditor("tex")
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  previewMaker = ReMaterialPreview::getInstance();
  
  uiUpdating = false;
}

QSize ReCheckersTextureEditor::sizeHint() const{
  return QSize(228, 368);
}

void ReCheckersTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  disconnect(mapping3D,   SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );
  disconnect(tex1, SIGNAL(editTexture(ReTexturePtr, QString&)), 
             this, SIGNAL(editTexture(ReTexturePtr, QString&)));
  disconnect(tex2, SIGNAL(editTexture(ReTexturePtr, QString&)), 
             this, SIGNAL(editTexture(ReTexturePtr, QString&)));

  removeTimer();

  ReAbstractTextureEditor::setDataModel(_model, mat);
  mapping3D->setDataModel(_model, mat);
  refreshUI();

  connect(tex1, SIGNAL(editTexture(ReTexturePtr, QString&)), 
          this, SIGNAL(editTexture(ReTexturePtr, QString&)));
  connect(tex2, SIGNAL(editTexture(ReTexturePtr, QString&)), 
          this, SIGNAL(editTexture(ReTexturePtr, QString&)));

  connect(mapping3D,   SIGNAL(mappingChanged()), this, SLOT(requestPreview()) );

  // Timer for the update of the texture preview. 
  // We don't want to call for a refresh of the texture preview for each
  // small edit of a numeric field. We start a timer and if there are no
  // other updates during a 1/2 second then the texture preview is requsted

  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(requestPreview()));

  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  requestPreview();
};

void ReCheckersTextureEditor::refreshUI() {
  if (uiUpdating) {
    return;
  }
  tex1->setTexture( model->getNamedValue("tex1").value<ReTexturePtr>(), material );
  tex2->setTexture( model->getNamedValue("tex2").value<ReTexturePtr>(), material );
}

void ReCheckersTextureEditor::requestPreview() {
  stopTimer();
  msgRenderingPreview->setText(tr("Rendering texture..."));
  ReTexturePtr tex = model->getTexture();
  ReLuxTextureExporterPtr exporter = ReLuxTextureExporterFactory::getExporter(tex);
  QString texture = exporter->exportTexture(tex, "Texture", true);

  connect( previewMaker, SIGNAL(previewReady(QString,QString,QImage*)), 
           this, SLOT(updatePreview(QString,QString,QImage*)) );
  ReMaterialPreview::getInstance()->sendPreviewRequest(
    "checkers","checkers", "-", texture, true, true
  );
}

void ReCheckersTextureEditor::updatePreview( QString matName, 
                                             QString previewID,
                                             QImage* preview  ) {
  if (matName != "checkers") {
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
