/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReLightEditor.h"

#include <QFileDialog>

#include "ReLight.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "RealityUI/RePresetSelector.h"


#define RL_COL_NAME     0
#define RL_COL_TYPE     1
#define RL_COL_GROUP    2
#define RL_COL_LIGHT_ON 3

ReLightEditor::ReLightEditor( QWidget* parent) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  gbLightProperties->setEnabled(false);

  connect(lightList, SIGNAL(itemSelectionChanged()), this, SLOT(lightSelected()));

  connect(lcmTemperature, SIGNAL(toggled(bool)), this, SLOT(updateColorModelEditor()));
  connect(lcmRgb, SIGNAL(toggled(bool)), this, SLOT(updateColorModelEditor()));
  connect(lcmPreset, SIGNAL(toggled(bool)), this, SLOT(updateColorModelEditor()));
  connect(gain, SIGNAL(valueChanged(double)), this, SLOT(updateGain(double)));
  connect(power, SIGNAL(valueChanged(double)), this, SLOT(updatePower(double)));
  connect(efficiency, SIGNAL(valueChanged(double)), this, SLOT(updateEfficiency(double)));
  connect(coneFeather, SIGNAL(valueChanged(double)), this, SLOT(updateConeFeather(double)));
  connect(coneAngle, SIGNAL(valueChanged(double)), this, SLOT(updateConeAngle(double)));
  connect(lightGroup, SIGNAL(textChanged(const QString&)), 
          this, SLOT(updateLightGroup(const QString&)));

  // Color temperature changed
  connect(colorTemperature, SIGNAL(valueChanged(int)), this, SLOT(updateColorTemperature(int)));
  // RGB color changed
  connect(lightRGB, SIGNAL(colorChanged()), this, SLOT(updateColorRGB()));
  // Light preset changed
  connect(btnLightPreset, SIGNAL(clicked()), this, SLOT(updatePreset()));
  // Light turned on/off
  connect(lightList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this, SLOT(updateLightOnOff(QTreeWidgetItem*,int)));

  connect(btnIESFile, SIGNAL(clicked()), this, SLOT(selectIESMap()));

  /*
   * IBL
   */
  connect(iblMapLatLong, SIGNAL(toggled(bool)), this, SLOT(updateIBLMapFormat()));
  connect(iblMapAngular, SIGNAL(toggled(bool)), this, SLOT(updateIBLMapFormat()));
  // RGB color changed
  connect(iblColor, SIGNAL(colorChanged()), this, SLOT(updateColorRGB()));
  connect(iblGain, SIGNAL(valueChanged(double)), this, SLOT(updateGain(double)));
  connect(gamma, SIGNAL(valueChanged(double)), this, SLOT(updateGamma(double)));
  connect(btnIBLMapChange, SIGNAL(clicked()), this, SLOT(updateIBLMap()));
  connect(YaxisRotation, SIGNAL(valueChanged(double)), this, SLOT(updateYRotation(double)));
  connect(iblFileName, SIGNAL(textChanged(const QString&)), this, SLOT(updateIBLFileName(const QString&)));

  // SUN
  connect(sunGain, SIGNAL(valueChanged(double)), this, SLOT(updateSun(double)));
  connect(sunTurbidity, SIGNAL(valueChanged(double)), this, SLOT(updateSun(double)));
  connect(sunSize, SIGNAL(valueChanged(double)), this, SLOT(updateSun(double)));
  connect(skyIntensity, SIGNAL(valueChanged(double)), this, SLOT(updateSun(double)));
  connect(skyTurbidity, SIGNAL(valueChanged(double)), this, SLOT(updateSun(double)));
  connect(newStyleSky, SIGNAL(toggled(bool)), this, SLOT(updateSkyStyle(bool)));

  // MESH
  connect(invertDirection, SIGNAL(toggled(bool)), this, SLOT(updateInvertDirection(bool)));
  connect(alphaChannel,    SIGNAL(toggled(bool)), this, SLOT(updateAlphaChannel(bool)));
  connect(btnToMaterial,   SIGNAL(clicked()),     this, SLOT(convertToMat()));

  /*
   * Light texture
   */
  connect(imageFile, SIGNAL(textChanged(const QString&)), this, SLOT(updateImageFile(const QString&)));
  connect(btnImageFile, SIGNAL(clicked()), this, SLOT(selectImageFile()));

  connect(leIESMapFile, SIGNAL(textChanged(const QString&)), this, SLOT(updateIESMap(const QString&)));

  lightList->setColumnWidth(RL_COL_NAME, 200);
  lightList->setColumnWidth(RL_COL_TYPE, 150);
  lightList->setColumnWidth(RL_COL_GROUP, 150);
  updatingUI = false;
}

void ReLightEditor::updateSun( double value ) {
  if (updatingUI) {
    return;
  }
  QString source = QObject::sender()->objectName();
  QVariant val(value);
  if (source == "sunGain") {
    source = "intensity";
  }
  else if ( source == "sunSize") {
    source = "sunScale";
  }
  currentLight->setNamedValue(source, val);
  emit lightChanged(currentLight->getID(), source, val);  
}

void ReLightEditor::updateSkyStyle( bool onOff ) {
  if (updatingUI) {
    return;
  }
  currentLight->setNewStyleSky(onOff);
  emit lightChanged(currentLight->getID(), "newStyleSky", onOff);
}


void ReLightEditor::addLightToList( ReLightPtr light ) {
  if (light.isNull()) {
    return;
  }
  QTreeWidgetItem* item = new QTreeWidgetItem(lightList);
  // ReLightPtr light = i.value();
  item->setText(RL_COL_NAME,light->getName());
  // Associate the light unique ID with the item for later recall
  item->setData(RL_COL_NAME, Qt::UserRole, light->getID());
  item->setText(RL_COL_TYPE,light->getGetTypeAsString());
  item->setText(RL_COL_GROUP,light->getLightGroup());
  item->setCheckState(RL_COL_LIGHT_ON, 
                      light->isLightOn() ? Qt::Checked : Qt::Unchecked);
  item->setToolTip(RL_COL_NAME, QString(tr("Internal ID: %1")).arg(light->getID()));
  lightList->addTopLevelItem(item);
}

void ReLightEditor::loadLightData() {
  updatingUI = true;
  lightList->clearSelection();
  lightList->clear();
  ReLightIterator i(RealitySceneData->getLights());

  while( i.hasNext() ){
    i.next();
    addLightToList(i.value());
  }

  updatingUI = false;
}

void ReLightEditor::updateLight( const QString& lightID ) {
  // Update the light 
  for( int i = 0; i < lightList->topLevelItemCount(); ++i )  {
    QTreeWidgetItem *item = lightList->topLevelItem( i );
    QString id = item->data(RL_COL_NAME, Qt::UserRole).toString();
    if (id == lightID) {
      ReLightPtr light = RealitySceneData->getLight(lightID);      
      item->setData(RL_COL_NAME, Qt::DisplayRole, light->getName());
      item->setText(RL_COL_TYPE,light->getGetTypeAsString());
      item->setCheckState(RL_COL_LIGHT_ON, 
                          light->isLightOn() ? Qt::Checked : Qt::Unchecked);
      item->setSelected(true);
      break;
    }
  }
  lightSelected();
}


void ReLightEditor::lightSelected() {
  auto items = lightList->selectedItems();
  QTreeWidgetItem* item = NULL;
  
  if (items.count() > 0) {
    item = items[0];
  }
  if (!item) {
    gbLightProperties->setEnabled(false);
    return;
  }

  gbLightProperties->setEnabled(true);
  QString lightName = item->data(RL_COL_NAME, Qt::UserRole).toString();
  currentLight = RealitySceneData->getLight(lightName);

  if (currentLight.isNull()) {
    return;
  }

  updatingUI = true;

  ReLightType lightType = currentLight->getType();
  // Is this an IBL light?
  if (lightType == IBLLight) {
    swLightProperties->setCurrentWidget(IBL);
    YaxisRotation->setValue(currentLight->getIBLRotationAngle());
    iblFileName->setText(currentLight->getIBLMapFileName());
    iblGain->setValue(currentLight->getIntensity());
    iblColor->setColor(currentLight->getColor());
    gamma->setValue(currentLight->getGamma());
    if (currentLight->getIBLMapFormat() == ReLight::LatLong) {
      iblMapLatLong->setChecked(true);
    }
    else {
      iblMapAngular->setChecked(true);
    }
  }
  else {
    swLightProperties->setCurrentWidget(nonIBL);
    // Is this the Sun?
    if (lightType == SunLight) {
      lightSubEditor->setCurrentWidget(sunLight);
      sunGain->setValue(currentLight->getIntensity());
      sunSize->setValue(currentLight->getSunScale());
      sunTurbidity->setValue(currentLight->getSunTurbidity());
      skyIntensity->setValue(currentLight->getSkyIntensity());
      skyTurbidity->setValue(currentLight->getSkyTurbidity());
      newStyleSky->setChecked(currentLight->hasNewStyleSky());
    }
    else {
      lightSubEditor->setCurrentWidget(nonSun);
    }

    btnToMaterial->setVisible(currentLight->isFromMaterial());
    gain->setValue(currentLight->getIntensity());
    power->setValue(currentLight->getPower());
    efficiency->setValue(currentLight->getEfficiency());
    leIESMapFile->setText(currentLight->getIesFileName());
    if (currentLight->getType() == SpotLight) {
      frAngle->show();
      frFeather->show();
      coneAngle->setValue(currentLight->getAngle());
      coneFeather->setValue(currentLight->getFeather());
    }
    else {
      frAngle->hide();
      frFeather->hide();
    }
    if (currentLight->getType() == MeshLight) {
      invertDirection->show();
      alphaChannel->show();
      invertDirection->setChecked(currentLight->getInvertedNormals());
      alphaChannel->setChecked(currentLight->hasAlphaChannel());
    }
    else {
      invertDirection->hide();    
      alphaChannel->hide();
    }
    // Color temperature
    lightGroup->setText(currentLight->getLightGroup());
    invertDirection->setChecked(currentLight->getInvertedNormals());
    switch( currentLight->getColorModel() ) {
      case ReLight::Temperature:
        lcmTemperature->setChecked(true);
        colorTemperature->setValue(currentLight->getColorTemperature());
        break;
      case ReLight::RGB:
        lcmRgb->setChecked(true);
        lightRGB->setColor(currentLight->getColor());
        break;
      case ReLight::Preset:
        lcmPreset->setChecked(true);
        QString desc = currentLight->getPresetDescription();
        QStringList presetDesc = desc.split('|');
        presetName->setText(presetDesc[0]);
        if (presetDesc.count() > 1) {
          presetDescription->setText(presetDesc[1]);        
        }
        break;
    }
    /*
     * Image map
     */
    imageFile->setText(currentLight->getTexture());
  }
  updateColorModelEditor();
  updatingUI = false;
}

void ReLightEditor::updateColorModelEditor() {
  if (lcmTemperature->isChecked()) {
    colorModelEditor->setCurrentWidget(temperatureEditor);
    currentLight->setColorModel(ReLight::Temperature);
    if (!updatingUI) {
      emit lightChanged(currentLight->getID(), "colorModel", ReLight::Temperature);
    }
  }
  else if (lcmRgb->isChecked()) {
    colorModelEditor->setCurrentWidget(rgbEditor);
    currentLight->setColorModel(ReLight::RGB);
    if (!updatingUI) {
      emit lightChanged(currentLight->getID(), "colorModel", ReLight::RGB);
    }
  }
  else if (lcmPreset->isChecked()) {
    colorModelEditor->setCurrentWidget(presetEditor);
    currentLight->setColorModel(ReLight::Preset);
    if (!updatingUI) {
      emit lightChanged(currentLight->getID(), "colorModel", ReLight::Preset);
    }
  }
};

void ReLightEditor::updateGain( double newVal ) {
  if (updatingUI) {
    return;
  }
  currentLight->setIntensity(newVal);
  emit lightChanged(currentLight->getID(), "intensity", newVal);
}

void ReLightEditor::updatePower( double newVal ) {
  if (updatingUI) {
    return;
  }
  currentLight->setPower(newVal);
  emit lightChanged(currentLight->getID(), "power", newVal);
}

void ReLightEditor::updateEfficiency( double newVal ) {
  if (updatingUI) {
    return;
  }
  currentLight->setEfficiency(newVal);
  emit lightChanged(currentLight->getID(), "efficiency", newVal);
}

void ReLightEditor::updateLightGroup(const QString& newVal ) {
  if (updatingUI) {
    return;
  }
  currentLight->setLightGroup(newVal);
  QTreeWidgetItem* item = lightList->currentItem();
  item->setText(RL_COL_GROUP, newVal);
  emit lightChanged(currentLight->getID(), "lightGroup", newVal);
}

void ReLightEditor::lightDeleted( const QString& lightID ) {
  int index = -1;
  for( int i = 0; i < lightList->topLevelItemCount(); ++i )  {
    QTreeWidgetItem *item = lightList->topLevelItem( i );
    QString id = item->data(RL_COL_NAME, Qt::UserRole).toString();
    if (id == lightID) {
      index = i;
      break;
    }    
  }

  QAbstractItemModel* model = lightList->model();
  if (index != -1) {
    model->removeRow(index);
  }
}

void ReLightEditor::updateColorTemperature(int newVal) {
  if (updatingUI) {
    return;
  }
  currentLight->setColorTemperature(newVal);
  emit lightChanged(currentLight->getID(), "colorTemperature", newVal);
}

void ReLightEditor::updateColorRGB() {
  if (updatingUI) {
    return;
  }
  QColor clr;
  if (currentLight->getType() == IBLLight) {
    clr = iblColor->getColor();
  }
  else {
    clr = lightRGB->getColor();
  }
  currentLight->setColor(clr);
  emit lightChanged(currentLight->getID(), "color", clr);
}

void ReLightEditor::updateConeAngle(double newVal) {
  if (updatingUI) {
    return;
  }
  currentLight->setAngle(newVal);
  emit lightChanged(currentLight->getID(), "angle", newVal);
};

void ReLightEditor::updateConeFeather(double newVal) {
  if (updatingUI) {
    return;
  }
  currentLight->setFeather(newVal);
  emit lightChanged(currentLight->getID(), "feather", newVal);
};

void ReLightEditor::updatePreset() {
  if (updatingUI) {
    return;
  }
  RePresetSelector presetSelector(this);
  presetSelector.setData(":/textResources/LightPresets.json");
  if ( presetSelector.exec() == QDialog::Rejected) {
    return;
  }
  QString presetCode = presetSelector.getValue();
  QString presetText = presetSelector.getText();
  QString presetDesc = presetSelector.getDescription();
  if (presetCode != "") {
    currentLight->setPresetCode(presetCode);
    QString textPlusDesc = QString("%1|%2").arg(presetText).arg(presetDesc);
    currentLight->setPresetDescription(textPlusDesc);
    presetName->setText(presetText);
    presetDescription->setText(presetDesc);
    emit lightChanged(currentLight->getID(), "presetCode", presetCode);
    emit lightChanged(currentLight->getID(), "presetDescription", textPlusDesc);
  }
}

void ReLightEditor::convertToMat() {
  // The light ID is made out of the object name and material name
  // separated by "::"
  QString lightID = currentLight->getID();
  QStringList idTokens = lightID.split("::");
  if (idTokens.count() < 2) {
    return;
  }
  emit convertToMaterial(idTokens[0], idTokens[1], lightID);
}


void ReLightEditor::updateLightOnOff(QTreeWidgetItem* item, int column) {
  if (updatingUI) {
    return;
  }
  if (!item) {
    return;
  }
  if (lightList->model()->rowCount() == 0) {
    return;
  }
  lightList->setCurrentItem(item);
  bool onOff;
  if (column == RL_COL_LIGHT_ON) {
    onOff  = (item->checkState(column) == Qt::Checked);
    currentLight->turnOnOff(onOff);
    emit lightChanged(currentLight->getID(), "isOn", onOff);
  }
}

void ReLightEditor::updateIESMap( const QString& newMapName ) {
  currentLight->setIesFileName(newMapName);
  emit lightChanged(currentLight->getID(), "iesFileName", newMapName);
}

void ReLightEditor::selectIESMap() {
  QFileInfo fInfo(leIESMapFile->text());

  QString fName = QFileDialog::getOpenFileName(
                    this, 
                    tr("Select a IES file"),
                    fInfo.absolutePath(),
                    tr("IES Files (*.ies *.txt)")
                  );
  if (fName != "") {
    leIESMapFile->setText(fName);
    currentLight->setIesFileName(fName);
    emit lightChanged(currentLight->getID(), "iesFileName", fName);
  }
}

// IBL Editor methods
void ReLightEditor::updateIBLMapFormat() {
  if (updatingUI) {
    return;
  }
  if (iblMapLatLong->isChecked()) {
    currentLight->setIBLMapFormat(ReLight::LatLong);
  }
  else if (iblMapAngular->isChecked()) {
    currentLight->setIBLMapFormat(ReLight::Angular);
  }
  emit lightChanged(currentLight->getID(), "mapFormat", currentLight->getIBLMapFormat());
}

void ReLightEditor::updateIBLMap() {
  QFileInfo fInfo(iblFileName->text());

  QString fName = QFileDialog::getOpenFileName(
                    this, 
                    tr("Select a file for IBL"),
                    fInfo.absolutePath(),
                    tr("IBL files (*.hdr *.exr *.png *.jpg *.tif *.tiff *.ibl)")
                  );
  if (fName != "") {
    updatingUI = true;
    iblFileName->setText(fName);
    currentLight->setIBLMapFileName(fName);
    currentLight->turnOnOff(true);
    iblMapLatLong->setChecked(true);
    emit lightChanged(currentLight->getID(), "mapFileName", fName);
    emit lightChanged(currentLight->getID(), "isOn", true);
    updateLight(currentLight->getID());
    updatingUI = false;
  }
}

void ReLightEditor::updateYRotation(double angle) {
  if (updatingUI) {
    return;
  }
  currentLight->setIBLRotationAngle(angle);
  emit lightChanged(currentLight->getID(), "rotationAngle", angle);  
}

void ReLightEditor::updateGamma( double newVal ) {
  if (updatingUI) {
    return;
  }
  currentLight->setGamma(newVal);
  emit lightChanged(currentLight->getID(), "gamma", newVal);  
}

void ReLightEditor::updateInvertDirection(bool isOn) {
  if (updatingUI) {
    return;
  }
  currentLight->setInvertedNormals(isOn);
  emit lightChanged(currentLight->getID(), "invertedNormals", isOn);
}

void ReLightEditor::updateAlphaChannel(bool isOn) {
  if (updatingUI) {
    return;
  }
  currentLight->setAlphaChannel(isOn);
  emit lightChanged(currentLight->getID(), "alphaChannel", isOn);  
}

void ReLightEditor::updateImageFile(const QString& fName) {
  if (updatingUI) {
    return;
  }
  currentLight->setTexture(fName);
  emit lightChanged(currentLight->getID(), "texture", fName);  
}

void ReLightEditor::selectImageFile() {
  QFileInfo fInfo(imageFile->text());

  QString fName = QFileDialog::getOpenFileName(
                    this,
                    tr("Select an image file"),
                    fInfo.absolutePath(),
                    tr("Image Files (*.jpg *.jpeg *.png *.tif)")
                  );
  if (fName != "") {
    imageFile->setText(fName);
  }
}

void ReLightEditor::updateIBLFileName(const QString& fName) {
  if (updatingUI) {
    return;
  }
  iblFileName->setText(fName);
  currentLight->setIBLMapFileName(fName);
  emit lightChanged(currentLight->getID(), "mapFileName", fName);  
}

void ReLightEditor::eraseAllLights() {
  updatingUI = true;
  lightList->clearSelection();
  lightList->clear();
  updatingUI = false;
}

