  /**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#include <QMessageBox>
#include "ReUniShaderSave.h"
#include "ReLogger.h"
#include "RealityBase.h"
#include "ReSceneDataGlobal.h"
#include "ReUniversalShaderSelector.h"
#include "exporters/lux/ReLuxMaterialExporterFactory.h" 
#include "ReTools.h"

//! The quality for the preview is adjusted with a granularity defined
//! by this constant
#define QUALITY_GRANULARITY 50 

//! This constant defines the time needed, in seconds, to calculate
//! 50 samples per pixel for the preview set a 200x200 pixels.
//! The time was obtained by rendering a preview on a 24 thread system
//! running at 3.3Ghz. The total time was 1:38 or 98 seconds. Let's
//! round them to 100. 100*24 => 2400, the time to run the same render
//! but with one thread or core. 2400/6 => 400, the time to render 
//! 50 samples with one core. 50 samples is the granularity for 
//! adjusting the render quality.

#define TIME_FOR_50_SPX 400

using namespace Reality;

ReUniShaderSave::ReUniShaderSave( ReMaterial* mat, QWidget* parent ) : 
  QDialog(parent),
  mat(mat)
{
  setupUi(this);
  shaderID = "";
  creatingPreview = false;
  refreshInterval = 5; // seconds
  // make sure that there are no doubts about the state of the file name
  previewFileName = ""; 
  // Preload the thumbnail to show that the material preview is
  // rendering
  renderingThumbNail.load(":/images/UniShaderRendering.png");

  teDescription->setAcceptRichText(false);
  innerSpherePresets.initComboBox(innerSphereMaterial);
  categoryPresets = QSharedPointer<ReShaderCategoryPreset>(
                        new ReShaderCategoryPreset(true)
                    );
  categoryPresets->initComboBox(categories);
  
  auto cfg = RealityBase::getConfiguration();
  leAuthor->setText(cfg->value(RE_CFG_AUTHOR_NAME,"").toString());
  auto acsel = ReAcsel::getInstance();
  QVariantMap data;
  if (acsel->findUniversalShader(shaderID, data)) {
    leName->setText(data.value("Description").toString());
    leAuthor->setText(data.value("Author").toString());
    teDescription->setText(data.value("Description").toString());
  }

  setPreviewQuality(previewQuality->value());
  connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchShader()));
  connect(btnMatPreview, SIGNAL(clicked()), this, SLOT(createMaterialPreview()));
  //! Previer quality slider control
  connect(previewQuality, SIGNAL(valueChanged(int)), 
          this, SLOT(setPreviewQuality(int)));
  //! The handler for the refresh timer
  connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(updatePreview()));  
};


void ReUniShaderSave::accept() {
  if (validate()) {
    QDialog::accept();
  }
}

void ReUniShaderSave::reject() {
  if (!renderProc.isNull() && !renderProc->isFinished()) {
    renderProc->killProcess();
  }
  refreshTimer.stop();
  QDialog::reject();
}

void ReUniShaderSave::showMsg( const QString& msg ) {
  QMessageBox::information( this, tr("Information"), msg);
}

bool ReUniShaderSave::validate() {
  if (leName->text().isEmpty()) {
    showMsg(tr("You need to enter a name for the shader"));
    leName->setFocus();
    return false;
  }
  if (teDescription->toPlainText().isEmpty()) {
    showMsg(tr("You need to enter a description"));
    teDescription->setFocus();
    return false;
  }
  if (leAuthor->text().isEmpty()) {
    showMsg(tr("You need to enter the author's name"));
    leAuthor->setFocus();
    return false;
  }
  if (categoryPresets->getID(categories->currentText()) == 0) {
    showMsg(tr("Please select a category for this shader"));
    categories->setFocus();
    return false;
  }
  return true;
}

void ReUniShaderSave::searchShader() {
  // Ask the user to select a shader set...
  ReUniversalShaderSelector shaderSelector(this);
  if ( shaderSelector.exec() == QDialog::Rejected ) {
    return;
  }

  shaderID = shaderSelector.getSelectedShaderID();
  auto acsel = ReAcsel::getInstance();
  QVariantMap shaderData;
  if (acsel->findUniversalShader(shaderID, shaderData)) {
    leName->setText(shaderData["Name"].toString());
    teDescription->setText(shaderData["Description"].toString());
    categories->setCurrentIndex(
      categoryPresets->getID(shaderData["CategoryName"].toString())
    );
    unsigned char* bitmap;
    int blobSize = 0;
    acsel->getUniversalShaderPreview(shaderID, bitmap, blobSize);
    if (blobSize > 0) {
      QPixmap p;
      p.loadFromData(bitmap, blobSize);
      matPreview->setPixmap(p);
      delete bitmap;
      previewFileName = "";
    }
  }
}

void ReUniShaderSave::setPreviewQuality(int newVal) {
  int numCores = getNumCPUs();
  estimatedRenderTime = newVal / QUALITY_GRANULARITY * TIME_FOR_50_SPX / numCores;
  lblEstimatedTime->setText(QString("%1.%2 min")
                              .arg(estimatedRenderTime/60)
                              .arg(estimatedRenderTime % 60));
  lblQuality->setText(QString(tr("Quality: %1 spx")).arg(newVal));
}

void ReUniShaderSave::updatePreview() {
  if (renderProc->isFinished()) {
    refreshTimer.stop();
    creatingPreview = false;
    progressBar->setValue(progressBar->maximum());
    btnMatPreview->setText(previewButtonOriginalText);
  }
  else {
    progressBar->setValue(progressBar->value()+1);
    btnMatPreview->setText(
      QString(tr("%1sec. Stop rendering...")).arg(stopwatch.elapsed() / 1000)
    );    
  }
  if (QFile(previewFileName).exists()) {
    matPreview->setPixmap(previewFileName);
  }
}

void ReUniShaderSave::createMaterialPreview() {
  if (creatingPreview) {
    creatingPreview = false;
    renderProc->killProcess();
    updatePreview();
    return;
  }
  auto config = RealityBase::getConfiguration();
  auto rBase = RealityBase::getRealityBase();
  QString contentDirKey = (rBase->getHostAppID() == DAZStudio ?
                           RE_CFG_STUDIO_CONTENT_DIR :
                           RE_CFG_POSER_CONTENT_DIR );
  contentDir = config->value( contentDirKey ).toString();
  QString templateSceneFileName = QString(
                                    "%1/LuxBall/RealityLuxBall_template.lxs"
                                  )
                                  .arg(contentDir);
  QString sceneFileName = QString("%1/LuxBall/RealityLuxBall.lxs")
                            .arg(contentDir);
  previewFileName = QString("%1/LuxBall/RealityLuxBall.png").arg(contentDir);
  QFile templateScene(templateSceneFileName);
  bool ok = false;
  if (!templateScene.open(QIODevice::ReadOnly)) {
    QMessageBox::information(
      this,
      tr("Information"),
      QString(
         tr("The scene file '%1', used for generating the material preview,"
         " cannot be found.\n"
         "Please re-install Reality to correct this issue")
      ).arg(templateSceneFileName)
    );
    return;
  }
  auto content = QString(templateScene.readAll());
  templateScene.close();
  QString matStr = exportMaterialToString(mat);
  // Get the choice for the inner sphere material
  auto presetID = innerSpherePresets.getID(innerSphereMaterial->currentText());
  matStr.replace(mat->getUniqueName(), "luxball");
  content.replace(":samples:", QString::number(previewQuality->value()));
  content.replace(":material:", matStr);
  if (presetID == ReInnerSpherePresets::SameAsOuterSphere) {
    matStr.replace("MakeNamedMaterial \"luxball\"", 
                   "MakeNamedMaterial \"luxball stand\"");
    content.replace(":inner material:", matStr);
  }
  else {
    content.replace(":inner material:", 
                    innerSpherePresets.getMaterial(
                      static_cast<ReInnerSpherePresets::InnerSphereMatType>(presetID)
                    )
                   );
  }
  QFile previewScene(sceneFileName);
  if (previewScene.open(QIODevice::WriteOnly)) {
    previewScene.write(content.toUtf8());
    previewScene.close();
    ok = true;
  }
  if (!ok) {
    RE_LOG_WARN() << "Error in creating the material preview";
    return;
  }
  // Add the subdivision and displacement definitions to the .lxo 
  // file
  ok = false;
  QString objFileName = QString("%1/LuxBall/RealityLuxBall.lxo")
                            .arg(contentDir);
  QString templateObjFileName = QString(
                                  "%1/LuxBall/RealityLuxBall_template.lxo"
                                )
                                .arg(contentDir);
  QFile templateObjFile(templateObjFileName);
  if (templateObjFile.open(QIODevice::ReadOnly)) {
    auto content = QString(templateObjFile.readAll());
    templateObjFile.close();
    // Obtain an exporter for the material and inject the subdivision
    // and displacement clauses, if present
    auto matExporter = ReLuxMaterialExporterFactory::getExporter(mat);

    content.replace(":subdivision:", matExporter->getSubdivision(mat));
    QString dispClause = matExporter->getDisplacementClause(mat);
    dispClause.replace(mat->getUniqueName(), "luxball");
    content.replace(":displacement:", dispClause);
    // Replace the name of the object used in the scene with "luxball"
    // so that the material is connected to the textures

    QFile objFile(objFileName);
    if (objFile.open(QIODevice::WriteOnly)) {
      objFile.write(content.toUtf8());
      objFile.close();
      ok = true;
    }
  }
  // Now render and show the preview
  if (ok) {
    renderProc = ReLuxRunnerPtr(new ReLuxRunner());
    auto result = renderProc->renderViaConsole(sceneFileName);
    if (result == ReLuxRunner::LR_NORMAL_EXIT) {
      QFile(previewFileName).remove();
      matPreview->setPixmap(renderingThumbNail);
      previewButtonOriginalText = btnMatPreview->text();
      btnMatPreview->setText(tr("Stop rendering..."));
      creatingPreview = true;
      stopwatch.start();
      refreshTimer.start(refreshInterval * 1000);
      progressBar->reset();
      progressBar->setMaximum(estimatedRenderTime/refreshInterval);
    }
  }
} 

