/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReOutputOptions.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "RealityBase.h"
#include "ReOpenCL.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReTools.h"
#include "actions/ReOutputOptionsActions.h"


ReOutputOptions::ReOutputOptions( QWidget* parent ) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  // Set the log levels for Lux
  luxLogLevels.initTable();
  luxLogLevels.initComboBox(cbbLuxLogLevel);
  // Set the table of geometry formats
  luxGeometryFormats.initTable();
  luxGeometryFormats.initComboBox(geometryFormat);
  // Set the image file formats
  imageFileFormats.initTable();
  imageFileFormats.initComboBox(imageFormat);

  // Initialize the list of Surface Integrators
  luxSIPresets.initTable();
  luxSIPresets.initComboBox(sceneType);

  // Initialize the list of Samplers
  renderSamplers.initTable();
  renderSamplers.initComboBox(sampler);

  config = RealityBase::getConfiguration();

  connect(alphaChannel, SIGNAL(toggled(bool)), this, SLOT(updateAlphaChannel(bool)));
  connect(cbbLuxLogLevel, SIGNAL(activated(const QString &)), this, SLOT(setLuxLogLevel(const QString &)));
  connect(gamma, SIGNAL(valueChanged(double)), this, SLOT(updateGamma(double)));
  connect(geometryFormat, SIGNAL(activated(int)), this, SLOT(updateGeometryFormat(int)));
  connect(imageFormat, SIGNAL(activated(int)), this, SLOT(updateImageFormat(int)));
  connect(imageFileName, SIGNAL(textChanged(const QString&)), this, SLOT(updateImageFileName(const QString&)));
  connect(noDisplacement, SIGNAL(toggled(bool)), this, SLOT(updateDisplacementFlag(bool)));
  connect(numThreads, SIGNAL(valueChanged(int)), this, SLOT(updateNumThreads(int)));
  connect(overwriteWarning, SIGNAL(toggled(bool)), this, SLOT(updateOverwriteWarning(bool)));
  connect(preserveLightHue, SIGNAL(toggled(bool)), this, SLOT(updateLightHueFlag(bool)));

  connect(sceneFrameSize, SIGNAL(valueChanged(int)), this, SLOT(updateFrameSize(int)));
  connect(sceneType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSceneType(int)));
  connect(maxSamples, SIGNAL(valueChanged(int)), this, SLOT(updateMaxSamples(int)));
  connect(FFRemoval, SIGNAL(valueChanged(int)), this, SLOT(updateFFRemoval(int)));

  connect(sampler, SIGNAL(activated(int)), this, SLOT(updateSampler(int)));

  connect(chbNoiseAware, SIGNAL(toggled(bool)), this, SLOT(updateNoiseAware(bool)));
  connect(btnImageName, SIGNAL(clicked()), this, SLOT(updateImageFileName()));

  // Acceleration options
  connect(cpuAcceleration, SIGNAL(toggled(bool)), this, SLOT(updateRenderMode(bool)));
  connect(OpenCLEnabled, SIGNAL(toggled(bool)), this, SLOT(updateRenderMode(bool)));
  connect(OCLBiased, SIGNAL(toggled(bool)), this, SLOT(updateBiasedRender(bool)));
  connect(CPUBiased, SIGNAL(toggled(bool)), this, SLOT(updateBiasedRender(bool)));
  connect(useSLG, SIGNAL(toggled(bool)), this, SLOT(updateRenderMode(bool)));
  connect(cpuSlow, SIGNAL(toggled(bool)), this, SLOT(updateRenderMode(bool)));
  
  connect(oclDevices, SIGNAL(itemChanged(QListWidgetItem*)), 
          this, SLOT(updateOCLDeviceStatus(QListWidgetItem*)));

  connect(OCLGroupSize, SIGNAL(valueChanged(int)), this, SLOT(updateOCLGroupSize(int)));

  connect(doTextureCollection, SIGNAL(toggled(bool)), this, SLOT(updateTextureCollection(bool)));
  connect(texSize025, SIGNAL(toggled(bool)), this, SLOT(updateTextureSize()));
  connect(texSize050, SIGNAL(toggled(bool)), this, SLOT(updateTextureSize()));
  connect(texSize1K, SIGNAL(toggled(bool)), this, SLOT(updateTextureSize()));
  connect(texSize2K, SIGNAL(toggled(bool)), this, SLOT(updateTextureSize()));
  connect(texSizeOriginal, SIGNAL(toggled(bool)), this, SLOT(updateTextureSize()));

  //
  // Surface Integrator editors
  //
  // SPPM
  connect(maxEyeDepth, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(maxPhotonDepth, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(photonsPerPass, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(startRadius, SIGNAL(valueChanged(double)), this, SLOT(updateSIValue(double)));
  connect(sppmAlpha, SIGNAL(valueChanged(double)), this, SLOT(updateSIValue(double)));
  // Path
  connect(maxPathLength, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(pathAuto, SIGNAL(toggled(bool)), this, SLOT(updateLS(bool)));
  connect(pathAll, SIGNAL(toggled(bool)), this, SLOT(updateLS(bool)));
  connect(pathSingle, SIGNAL(toggled(bool)), this, SLOT(updateLS(bool)));
  // ExPhotonMap
  connect(directPhotons, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(indirectPhotons, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(causticPhotons, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(expmMaxDepth, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(expmMaxPhotonDepth, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(expmDirectLighting, SIGNAL(toggled(bool)), this, SLOT(updateExPMRendering(bool)));
  connect(expmPath, SIGNAL(toggled(bool)), this, SLOT(updateExPMRendering(bool)));
  // Bidir
  connect(cameraPasses, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(lightPasses, SIGNAL(valueChanged(int)), this, SLOT(updateSIValue(int)));
  connect(bidirAuto, SIGNAL(toggled(bool)), this, SLOT(updateLS(bool)));
  connect(bidirAll, SIGNAL(toggled(bool)), this, SLOT(updateLS(bool)));
  connect(bidirSingle, SIGNAL(toggled(bool)), this, SLOT(updateLS(bool)));


  // Clicking on the list of groups
  // connect(parameterGroups, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), 
  //         this, SLOT(parameterGroupSelected(QListWidgetItem*)));
  connect(btnOutput, SIGNAL(clicked()), this, SLOT(parameterGroupSelected()));
  connect(btnConfiguration, SIGNAL(clicked()), this, SLOT(parameterGroupSelected()));
  connect(btnRenderer, SIGNAL(clicked()), this, SLOT(parameterGroupSelected()));
  connect(btnTexCollection, SIGNAL(clicked()), this, SLOT(parameterGroupSelected()));
  connect(btnFrameRange, SIGNAL(clicked()), this, SLOT(parameterGroupSelected()));

  // Render frame button
  connect(btnRenderFrame, SIGNAL(clicked()), this, SIGNAL(renderFrame()));

  // Render animation
  connect(btnRenderAnimation, SIGNAL(clicked()), this, SLOT(renderAnimationWithRange()));
  
  // Render as statue
  connect(chbAsStatue, SIGNAL(toggled(bool)), this, SLOT(updateRenderAsStatue(bool)));

  btnOutput->setChecked(true);

  updatingUI = false;
  updateUI();
}

QSize ReOutputOptions::sizeHint() const {
  return QSize(821, 442); 
}

void ReOutputOptions::parameterGroupSelected() {
  if (updatingUI) {
    return;
  }

  QString source = QObject::sender()->objectName();

  if (source == "btnOutput") {
    renderOptions->setCurrentWidget(ro_Output);
  }
  else if (source == "btnConfiguration") {
    renderOptions->setCurrentWidget(ro_sceneType);
  }
  else if (source == "btnTexCollection") {
    renderOptions->setCurrentWidget(ro_textureCollection);
  }
  else if (source == "btnFrameRange") {
    int animStartFrame, animEndFrame, animFps;
    RealitySceneData->getAnimationLimits(animStartFrame, animEndFrame, animFps);
    hostStartFrame->setText(QString::number(animStartFrame));
    hostEndFrame->setText(QString::number(animEndFrame));
    fps->setText(QString::number(animFps));
    renderOptions->setCurrentWidget(ro_animation);
  }
  else if (source == "btnRenderer") {
    renderOptions->setCurrentWidget(ro_Renderer);
  }
}

void ReOutputOptions::updateUI() {
  updatingUI = true;
  // Starting with Reality 4.1 we don't allow to change the path for
  // LuxRender
  btnSetLuxPath->hide();

  luxLocation->setText(RealityBase::getRealityBase()->getRendererPath(LuxRender));
  luxVersion->setText("");
  imageFileName->setText(toNativeFilePath(RealitySceneData->getImageFileName()));
  gamma->setValue(RealitySceneData->getGamma());
  sceneFrameSize->setValue(RealitySceneData->getFrameMultiplier()*100);
  showFrameSize();
  preserveLightHue->setChecked(RealitySceneData->getPreserveLightHue());
  alphaChannel->setChecked(RealitySceneData->hasAlphaChannel());
  FFRemoval->setValue(RealitySceneData->getOutlierRejection());
  chbNoiseAware->setChecked(RealitySceneData->isNoiseAwareOn());

  // Set the number of rendering threads 
  if (config->value(RE_CFG_KEEP_UI_RESPONSIVE).toBool()) {
    int threads = getNumCPUs() * RE_CFG_PERCENT_CORES_FOR_RENDERING;
    numThreads->setValue(threads);
    RealitySceneData->setNumThreads(threads);
    emit outputDataChanged();
  }
  else {
    numThreads->setValue(getNumCPUs());
  }
  geometryFormat->setCurrentIndex(
    geometryFormat->findText(luxGeometryFormats.getValue(RealitySceneData->getGeometryFormat()))
  );
  imageFormat->setCurrentIndex(
    imageFormat->findText(
      imageFileFormats.getValue(RealitySceneData->getImageFileFormat())
    )
  );
  cbbLuxLogLevel->setCurrentIndex(
    cbbLuxLogLevel->findText(luxLogLevels.getValue(RealitySceneData->getLuxLogLevel()))
  );
  // parameterGroups->setCurrentRow(0);
  noDisplacement->setChecked(!RealitySceneData->isDisplacementEnabled());
  overwriteWarning->setChecked(RealitySceneData->getOverwriteWarning());
  ReSurfaceIntegratorType siType = RealitySceneData->getSurfaceIntegrator()->getType();

  sampler->setCurrentIndex(
    sampler->findText(
      renderSamplers.getValue( RealitySceneData->getSampler() )
    )
  );

  sceneType->setCurrentIndex(
   sceneType->findText(luxSIPresets.getValue(siType))
  );
  setSIEditor();
  maxSamples->setValue(RealitySceneData->getMaxSPX());
  OCLGroupSize->setValue(RealitySceneData->getOCLGroupSize());
    
  // Set the OCL devices
  ReOpenCL* ocl = ReOpenCL::getInstance();
  oclDevices->clear();
  // OCL is active for this machine
  if (ocl->isEnabled()) {
    OpenCLEnabled->setEnabled(true);
    oclDevices->setEnabled(true);
    lblOpenCL_status->setText(QString(tr("Yes ( %1 )"))
                                .arg(ocl->getPlatformList<QStringList>().join(", ")));
    auto deviceList = ocl->getDeviceList();
    RealitySceneData->setNumOCLDevices(deviceList.count());

    // Send the information to the host-side
    emit outputDataChanged();

    bool isBiased = RealitySceneData->getOCLBias() != 0;
    OCLBiased->setChecked(isBiased);

    foreach( ReOCLDevicePtr oclDev, deviceList ) {
      auto itm = new QListWidgetItem(oclDev->getLabel(), oclDevices);
      itm->setData(Qt::UserRole, oclDev->getNumber());
      itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
      if (isBiased && (oclDev->getType() == ReOCLDevice::RE_OCL_CPU)) {
        itm->setCheckState(Qt::Unchecked);
        RealitySceneData->setOCLDeviceFlag(oclDev->getNumber(), false);
      }
      else {
        itm->setCheckState(Qt::Checked);
        RealitySceneData->setOCLDeviceFlag(oclDev->getNumber(), true);
      }
      itm->setToolTip(QString("%1 - %2").arg(oclDev->getName()).arg(oclDev->getTypeAsString()));
    }
  }
  else {
    OpenCLEnabled->setEnabled(false);
    oclDevices->setEnabled(false);
  }

  // Determine which acceleration option is on
  if (RealitySceneData->isOCLRenderingON()) {
    OpenCLEnabled->setChecked(true);
    OCLBiased->setChecked(RealitySceneData->getOCLBias());
  }
  else {
    oclDevices->setEnabled(false);    
    if (RealitySceneData->cpuAccelerationEnabled()) {
      cpuAcceleration->setChecked(true);
      CPUBiased->setChecked(RealitySceneData->getCPUBias());
    }
    else {
      cpuSlow->setChecked(true);
    }
  }
  OCLBiased->setEnabled(OpenCLEnabled->isChecked());
  CPUBiased->setEnabled(cpuAcceleration->isChecked());

  bool slgFound = !RealityBase::getRealityBase()->getRendererPath(SLG).isEmpty();
  if (!(ocl->isEnabled() && slgFound)) {
    useSLG->hide();
  } 

  //!!! Temporary: change when fixed
  useSLG->hide();

  doTextureCollection->setChecked(RealitySceneData->hasTextureCollection());
  frTexSize->setEnabled(doTextureCollection->isChecked());

  ReTextureSize texSize = RealitySceneData->getTextureSize();
  switch( texSize ) {
    case T_256:
      texSize025->setChecked(true);
      break;
    case T_512:
      texSize050->setChecked(true);
      break;
    case T_1K:
      texSize1K->setChecked(true);
      break;
    case T_2K:
      texSize2K->setChecked(true);
      break;
    case T_ORIGINAL:
      texSizeOriginal->setChecked(true);
      break;
  }
  chbAsStatue->setChecked(RealitySceneData->isRenderAsStatueEnabled());
  // if (currentParameterGroup) {
  //   parameterGroups->setCurrentItem(currentParameterGroup);
  // }
  updatingUI = false;
}

void ReOutputOptions::showFrameSize() {
  float frameMultiplier = RealitySceneData->getFrameMultiplier();
  lblActualFrameSize->setText(QString("%1x%2")
                              .arg(RealitySceneData->getWidth()*frameMultiplier)
                              .arg(RealitySceneData->getHeight()*frameMultiplier));

}


void ReOutputOptions::setSIEditor() {
  
  ReSurfaceIntegratorPtr si = RealitySceneData->getSurfaceIntegrator();
  QVariantMap vals = si->getValues();
  switch( luxSIPresets.getID(sceneType->currentText()) ) {
    case SI_BIDIR:
      siEditor->setCurrentWidget(siBidirCfg);
      cameraPasses->setValue(vals["eyeDepth"].toInt());
      lightPasses->setValue(vals["lightDepth"].toInt());
      if (vals["lightStrategy"] == "auto") {
        bidirAuto->setChecked(true);
      }
      else if (vals["lightStrategy"] == "all") {
        bidirAll->setChecked(true);
      }
      else {
        bidirSingle->setChecked(true);
      }
      break;
    case SI_PATH:
      siEditor->setCurrentWidget(siPathCfg);
      maxPathLength->setValue(vals["maxDepth"].toInt());
      if (vals["lightStrategy"] == "auto") {
        pathAuto->setChecked(true);
      }
      else if (vals["lightStrategy"] == "all") {
        pathAll->setChecked(true);
      }
      else {
        pathSingle->setChecked(true);
      }
      break;
    case SI_EXPM:
      siEditor->setCurrentWidget(siExPhotonMapCfg);
      directPhotons->setValue(vals["directPhotons"].toInt());
      indirectPhotons->setValue(vals["indirectPhotons"].toInt());
      causticPhotons->setValue(vals["causticPhotons"].toInt());
      expmMaxDepth->setValue(vals["maxDepth"].toInt());
      expmMaxPhotonDepth->setValue(vals["maxPhotonDepth"].toInt());
      if (vals["directLighting"].toBool()) {
        expmDirectLighting->setChecked(true);
      }
      else {
        expmPath->setChecked(true);
      }
      break;
    case SI_SPPM:
      siEditor->setCurrentWidget(siSPPMCfg);
      maxEyeDepth->setValue(vals["maxEyeDepth"].toInt());
      maxPhotonDepth->setValue(vals["maxPhotonDepth"].toInt());
      photonsPerPass->setValue(vals["photonsPerPass"].toInt());
      startRadius->setValue(vals["startRadius"].toInt());
      sppmAlpha->setValue(vals["alpha"].toDouble());
      break;
  }
}

/*
void ReOutputOptions::updateSceneFileName( const QString& val ) {
  if (updatingUI) {
    return;
  }
  QString val2 = val;

  QFileInfo fi(val2);
  if (fi.suffix() != "lxs") { 
    val2 = QString("%1/%2.lxs").arg(fi.absolutePath()).arg(fi.baseName());
    updatingUI = true;
    sceneFileName->setText(val2);
    updatingUI = false;
  }

  RealityBase::getUndoStack()->push(
    new SceneFileAction(val2, this)
  );
  emit outputDataChanged();
}
*/

void ReOutputOptions::updateImageFileName( const QString& val ) {
  if (updatingUI) {
    return;
  }
  RealityBase::getUndoStack()->push(
    new ImageFileAction(val, this)
  );
  emit outputDataChanged();
}

void ReOutputOptions::updateImageFileName() {
  QFileInfo fInfo(imageFileName->text());
  QString fName = QFileDialog::getSaveFileName(
                                this, 
                                tr("Select an Image file"),
                                fInfo.absolutePath(),
                                tr("Image files (*.png *.jpg *.tga *.exr)")
                              );

  if (fName != "") {
    updateImageFileName(fName);
    showImageFileName(fName);
  }
}


void ReOutputOptions::updateAlphaChannel( bool val ) {
  if (updatingUI) {
    return;
  }
  RealityBase::getUndoStack()->push(new AlphaChannelAction(val, this));
  emit outputDataChanged();
}


void ReOutputOptions::updateBiasedRender( bool isON ) {
  if (updatingUI) {
    return;
  }
  QString source = QObject::sender()->objectName();
  if ( source == "OCLBiased") {
    RealitySceneData->setOCLBias( isON ? 1 : 0 );
  }  
  else if ( source == "CPUBiased") {
    RealitySceneData->setCPUBias( isON ? 1 : 0 );
  }  

  updateUI();
}

// We discard the cases where isON is false because we are only interested
// in the events that activate a flag. Since this method is called when a 
// radiobutton is changed, each change actually generates two calls: one with
// isON == false for the radiobutton that becomes unchecked, and one with isON == true
// for the radiobutton that is checked. We are only interested in the latter.
void ReOutputOptions::updateRenderMode( bool isON ) {
  // Don't change the data if we are simply refreshing the UI
  if (updatingUI) {
    return;
  }
  if (!isON) {
    return;
  }

  RealitySceneData->setCpuAcceleration(false);

  QString source = QObject::sender()->objectName();
  bool OCLRendering = false;

  RealitySceneData->setRenderer(ReRenderers::LuxRender);
  if (source == "useSLG") {
    RealitySceneData->setRenderer(ReRenderers::SLG);
    OCLRendering = true;
  }
  else if (source == "OpenCLEnabled") {
    OCLRendering = true;
    RealitySceneData->setCpuAcceleration(false);
    RealitySceneData->setOCLRendering(true);
  }
  else if (source == "cpuAcceleration") {
    RealitySceneData->setCpuAcceleration(true);
    RealitySceneData->setOCLRendering(false);
    RealitySceneData->setCpuAcceleration(true);
  }
  else if (source == "cpuSlow") {
    RealitySceneData->setCpuAcceleration(false);
    RealitySceneData->setOCLRendering(false);
    RealitySceneData->setCpuAcceleration(false);
  }

  if (OCLRendering) {
    RealitySceneData->setOCLRendering(isON);
    if ( !doTextureCollection->isChecked() ) {
      doTextureCollection->setChecked(true);
      texSize1K->setChecked(true);
    }
    // Switch the sampler to Sobol
    RealitySceneData->setSampler(sobol);
  }
  else {
    RealitySceneData->setSampler(metropolis);
    if (cpuAcceleration->isChecked()) {
      RealitySceneData->setSurfaceIntegrator(SI_PATH);
    }
    else {
      RealitySceneData->setSurfaceIntegrator(SI_BIDIR);
    }
  }
  updateUI();
  emit outputDataChanged();
}

void ReOutputOptions::updateGamma( double val ) {
  if (updatingUI) {
    return;
  }
  RealityBase::getUndoStack()->push(new GammaAction(val, this));
  emit outputDataChanged();
}

void ReOutputOptions::updateNumThreads(int val) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setNumThreads(val);
  emit outputDataChanged();
}

void ReOutputOptions::updateMaxSamples(int val) {
  if (updatingUI) {
    return;
  }
  if (RealityBase::getConfiguration()
      ->value(RE_CFG_MAX_SAMPLES_FIRST_TIME).toBool()) 
  {
    RealityBase::getConfiguration()
      ->setValue(RE_CFG_MAX_SAMPLES_FIRST_TIME, false); 
    if ( QMessageBox::question(
           this, 
           tr("Question"), 
           tr(
             "Setting the \"Max Samples\" parameter higher than zero will stop "
             "the rendering after the given number of samples per pixel have been "
             "computed. This can dramatically reduce the quality of an image and it's used "
             "generally only for animations.\n\n"
             "For more information please read the Reality User's Guide.\n\n"
             "Are you sure that you want to set this parameter now? "
           ),
           QMessageBox::Yes | QMessageBox::No
        ) == QMessageBox::No )
    {
      maxSamples->setValue(0);
      return;
    }
  }
  RealitySceneData->setMaxSPX(val);
  emit outputDataChanged();
}

void ReOutputOptions::updateFFRemoval(int val) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setOutlierRejection(val);
  emit outputDataChanged();
}

void ReOutputOptions::updateNoiseAware(bool val) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setNoiseAware(val);
  emit outputDataChanged();
}

/** 
 * Called when the user changed the spinner for the frame size %
 */
void ReOutputOptions::updateFrameSize( int newPercent ) {
  if (updatingUI) {
    return;
  }

  float decimalFraction = newPercent/100.0;

  RealityBase::getUndoStack()->push(new FrameMultiplierAction(decimalFraction, this));
  showFrameSize();
  emit outputDataChanged();  
}

void ReOutputOptions::updateGeometryFormat(int index) {
  if (updatingUI) {
    return;
  }

  GeometryFileFormat value = static_cast<GeometryFileFormat>(
    luxGeometryFormats.getID(geometryFormat->itemText(index))
  );
  RealityBase::getUndoStack()->push(new GeometryFormatAction(value, this));
  emit outputDataChanged();
}

void ReOutputOptions::setImageExtensionForFormat( ExportImageFileFormat newFormat ) {
  QString extension;
  switch( newFormat ) {
    case PNG:
      extension = "png";
      break;
    case EXR:
      extension = "exr";
      break;
    case TGA:
      extension = "tga";
      break;      
  }

  QFileInfo imageInfo( imageFileName->text() );
  QString newFileName;
  newFileName = QString("%1/%2.%3")
                  .arg(imageInfo.absolutePath())
                  .arg(imageInfo.baseName())
                  .arg(extension);
  imageFileName->setText(newFileName);
  RealitySceneData->setImageFileName(newFileName);
  emit outputDataChanged();
  emit outputDataChanged();
}

void ReOutputOptions::updateImageFormat(int index) {
  if (updatingUI) {
    return;
  }

  auto value = static_cast<ExportImageFileFormat>(
    imageFileFormats.getID(imageFormat->itemText(index))
  );
  RealityBase::getUndoStack()->push(new ImageFileFormatAction(value, this));
  emit outputDataChanged();
}

void ReOutputOptions::updateDisplacementFlag( bool onOff ) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setEnableDisplacement(!onOff);
  emit outputDataChanged();
}

void ReOutputOptions::updateOverwriteWarning( bool onOff ) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setOverwriteWarning(onOff);
  emit outputDataChanged();
}


void ReOutputOptions::updateLightHueFlag( bool onOff ) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setPreserveLightHue(onOff);
  emit outputDataChanged();
}

void ReOutputOptions::updateOCLGroupSize(int newVal) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setOCLGroupSize(newVal);
  emit outputDataChanged();
}

void ReOutputOptions::updateOCLDeviceStatus(QListWidgetItem* item ) {
  if (updatingUI) {
    return;
  }
  if (!item) {
    return;
  }

  quint16 devNo = static_cast<quint16>(item->data(Qt::UserRole).toInt());
  RealitySceneData->setOCLDeviceFlag(devNo, item->checkState() & Qt::Checked);
  emit outputDataChanged();
}

void ReOutputOptions::updateTextureCollection(bool isOn) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setTextureCollection(isOn);
  frTexSize->setEnabled(isOn);
  
  emit outputDataChanged();  
}


void ReOutputOptions::updateTextureSize() {
  if (updatingUI) {
    return;
  }
  QString source = QObject::sender()->objectName();

  if ( source == "texSize025") {
    RealitySceneData->setTextureSize(Reality::T_256);    
  }
  else if ( source == "texSize050") {
    RealitySceneData->setTextureSize(Reality::T_512);    
  }
  else if ( source == "texSize1K") {
    RealitySceneData->setTextureSize(Reality::T_1K);    
  }
  else if ( source == "texSize2K") {
    RealitySceneData->setTextureSize(Reality::T_2K);
  }
  else {
    RealitySceneData->setTextureSize(Reality::T_ORIGINAL);
  }
  emit outputDataChanged();
}


void ReOutputOptions::updateSIValue( int newVal ) {
  if (updatingUI) {
    return;
  }
  QString source = QObject::sender()->objectName();

  ReSurfaceIntegratorPtr si = RealitySceneData->getSurfaceIntegrator();
  if (source == "maxEyeDepth") {
    si->setNamedValue("maxEyeDepth", newVal);
  }
  else if (source == "expmMaxPhotonDepth") {
    si->setNamedValue("maxPhotonDepth", newVal);
  }
  else if (source == "photonsPerPass") {
    si->setNamedValue("photonsPerPass", newVal);
  }
  else if (source == "maxPhotonDepth") {
    si->setNamedValue("maxPhotonDepth", newVal);
  }
  else if (source == "maxPathLength") {
    si->setNamedValue("maxDepth", newVal);
  }
  else if (source == "directPhotons") {
    si->setNamedValue("directPhotons", newVal);
  }
  else if (source == "indirectPhotons") {
    si->setNamedValue("indirectPhotons", newVal);
  }
  else if (source == "causticPhotons") {
    si->setNamedValue("causticPhotons", newVal);
  }
  else if (source == "expmMaxDepth") {
    si->setNamedValue("maxDepth", newVal);
  }
  else if (source == "expmMaxPhotonDepth") {
    si->setNamedValue("maxPhotonDepth", newVal);
  }
  else if (source == "cameraPasses") {
    si->setNamedValue("eyeDepth", newVal);
  }
  else if (source == "lightPasses") {
    si->setNamedValue("lightDepth", newVal);
  }
  emit outputDataChanged();  
}

void ReOutputOptions::updateSIValue( double newVal ) {
  if (updatingUI) {
    return;
  }
  QString source = QObject::sender()->objectName();

  ReSurfaceIntegratorPtr si = RealitySceneData->getSurfaceIntegrator();
  if (source == "startRadius") {
    si->setNamedValue("startRadius", newVal);
  }
  else if (source == "sppmAlpha") {
    si->setNamedValue("alpha", newVal);
  }
  emit outputDataChanged();    
}

void ReOutputOptions::updateLS( bool isOn ) {
  if (updatingUI) {
    return;
  }
  QString source = QObject::sender()->objectName();

  ReSurfaceIntegratorPtr si = RealitySceneData->getSurfaceIntegrator();
  if ( (source == "pathAuto" || source == "bidirAuto") && isOn ) {
    si->setNamedValue("lightStrategy", "auto");
  }  
  else if ( (source == "pathAll" || source == "bidirAll") && isOn ) {
    si->setNamedValue("lightStrategy", "all");
  }  
  else if ( (source == "pathSingle" || source == "bidirSingle") && isOn ) {
    si->setNamedValue("lightStrategy", "one");
  }  
  emit outputDataChanged();  
}

void ReOutputOptions::updateExPMRendering( bool isOn ) {
  if (updatingUI) {
    return;
  }
  ReSurfaceIntegratorPtr si = RealitySceneData->getSurfaceIntegrator();
  si->setNamedValue("directLighting", expmDirectLighting->isChecked());
  emit outputDataChanged();  
}

void ReOutputOptions::updateSceneType( int newIndex ) {
  if (updatingUI) {
    return;
  }
  ReSurfaceIntegratorType si = static_cast<ReSurfaceIntegratorType>(
    luxSIPresets.getID(sceneType->currentText())
  );
  if (si != SI_PATH && RealitySceneData->getOCLBias()) {
    if ( QMessageBox::information(
           this, 
           tr("Invalid choice"), 
           tr("The scene type must be Monodirectional when using \"Extra Boost\"")
         ) ) 
    {
      updatingUI = true;
      sceneType->setCurrentIndex(
        sceneType->findText(luxSIPresets.getValue(SI_PATH))
      );
      updatingUI = false;
      return;
    }
  }
  RealitySceneData->setSurfaceIntegrator(si);
  setSIEditor();
  emit outputDataChanged();
}

/*
void ReOutputOptions::updateSceneFileName() {
  QFileInfo fInfo(imageFileName->text());
  QString fName = QFileDialog::getSaveFileName(
                                this, 
                                tr("Select an name for the scene file"),
                                fInfo.absolutePath(),
                                tr("Lux Scene (*.lxs)")
                              );

  if (fName != "") {
    QFileInfo fi(fName);
    if (fi.suffix() != "lxs") { 
      fName = QString("%1/%2.lxs").arg(fi.absolutePath()).arg(fi.baseName());
    }
    RealitySceneData->setSceneFileName(fName);
    showSceneFileName(fName);
  }
}
*/

void ReOutputOptions::updateSampler( int index ) {
  if (updatingUI) {
    return;
  }

  RenderSampler newSampler = static_cast<RenderSampler>(
    renderSamplers.getID(sampler->itemText(index))
  );
  RealitySceneData->setSampler(newSampler);
  emit outputDataChanged();
}

void ReOutputOptions::setLuxLogLevel(const QString & levelStr) {
  if (updatingUI) {
    return;
  }
  int level = luxLogLevels.getID(levelStr);
  if (level != -1) {
    RealitySceneData->setLuxLogLevel(static_cast<LUX_LOG_LEVEL>(level));
    emit outputDataChanged();
  }
}

void ReOutputOptions::updateRenderAsStatue( bool onOff ) {
  if (updatingUI) {
    return;
  }
  RealitySceneData->setRenderAsStatue(onOff);
  emit outputDataChanged();
}

/*
void ReOutputOptions::showSceneFileName(QString val) {
  sceneFileName->setText(toNativeFilePath(val));
  emit outputDataChanged();
}
*/

void ReOutputOptions::showImageFileName(QString val) {
  updatingUI = true;
  imageFileName->setText(toNativeFilePath(val));
  updatingUI = false;
  emit outputDataChanged();
}


void ReOutputOptions::showGamma( const double newGamma ){
  updatingUI = true;
  gamma->setValue(newGamma);
  updatingUI = false;
  emit outputDataChanged();  
};

void ReOutputOptions::showAlphaChannel( const bool newAlpha ){
  updatingUI = true;
  alphaChannel->setChecked(newAlpha);
  updatingUI = false;
  emit outputDataChanged();  
};
/*
void ReOutputOptions::showGPUSupport( const bool newVal ){
  updatingUI = true;
  // GPUAcceleration->setChecked(newVal);
  updatingUI = false;
  emit outputDataChanged();  
};
*/

void ReOutputOptions::showMaxSPX( const quint32 newVal ){
  updatingUI = true;
  maxSamples->setValue(newVal);
  updatingUI = false;
  emit outputDataChanged();  
};

void ReOutputOptions::showOutlierRejection( const quint16 newVal ){
  updatingUI = true;
  FFRemoval->setValue(newVal);
  updatingUI = false;
  emit outputDataChanged();  
};

void ReOutputOptions::showImageFileFormat( const ExportImageFileFormat newVal ){
  updatingUI = true;
  imageFormat->setCurrentIndex(
    imageFormat->findText(
      imageFileFormats.getValue(RealitySceneData->getImageFileFormat())
    )
  );
  updatingUI = false;
  emit outputDataChanged();  
};

void ReOutputOptions::showPreserveLightHue( const bool newVal ){
  updatingUI = true;
  preserveLightHue->setChecked(newVal);
  updatingUI = false;
  emit outputDataChanged();  
};

void ReOutputOptions::showFrameMultiplier( const float newVal ){
  updatingUI = true;
  sceneFrameSize->setValue(newVal*100);
  updatingUI = false;
  emit outputDataChanged();  
};

void ReOutputOptions::showGeometryFormat( const GeometryFileFormat newVal ){
  updatingUI = true;
  // 0 => Binary PLY, 1 Text PLY, 2 Lux Native
  switch(newVal) {
    case BinaryPLY:
      geometryFormat->setCurrentIndex(0);
      break;
    case TextPLY:
      geometryFormat->setCurrentIndex(1);
      break;
    case LuxNative:
      geometryFormat->setCurrentIndex(2);
      break;
  }
  updatingUI = false;
  emit outputDataChanged();  
};

void ReOutputOptions::renderAnimationWithRange() {
  emit renderAnimation(startFrame->value(), endFrame->value());
}


