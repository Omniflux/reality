/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include <QJson/Parser>

#include "ReGlassEditor.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReVolumes.h"


ReGlassEditor::ReGlassEditor(QWidget* parent) :  ReMaterialEditor(parent) {
  setupUi(this);
  inSetup = false;
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  ior->setKeyboardTracking(false);
  thinFilmIOR->setKeyboardTracking(false);
  thinFilmThickness->setKeyboardTracking(false);
  uRoughness->setKeyboardTracking(false);
  vRoughness->setKeyboardTracking(false);
  cauchyB->setKeyboardTracking(false);
  clarity->setKeyboardTracking(false);



  // connect(reflection,   SIGNAL(editTexture(ReTexturePtr, QString&)), 
  //         this,         SIGNAL(editTexture(ReTexturePtr, QString&)));
  
  // connect(transmission, SIGNAL(editTexture(ReTexturePtr, QString&)), 
  //         this,         SIGNAL(editTexture(ReTexturePtr, QString&)));

  KrAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      reflection, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KtAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      transmission, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );

  connect(standardGlass,      SIGNAL(toggled(bool)),        this, SLOT(updateGlassType()));
  connect(volumetricGlass,    SIGNAL(toggled(bool)),        this, SLOT(updateGlassType()));
  connect(architecturalGlass, SIGNAL(toggled(bool)),        this, SLOT(updateGlassType()));
  connect(frostedGlass,       SIGNAL(toggled(bool)),        this, SLOT(updateGlassType()));
  connect(ior,                SIGNAL(valueChanged(double)), this, SLOT(updateIOR(double)));
  connect(thinFilmIOR,        SIGNAL(valueChanged(double)), this, SLOT(updateThinFilmIOR(double)));
  connect(thinFilmThickness,  SIGNAL(valueChanged(double)), this, SLOT(updateThinFilmThickness(double)));
  connect(uRoughness,         SIGNAL(valueChanged(double)), this, SLOT(updateURoughness(double)));
  connect(vRoughness,         SIGNAL(valueChanged(double)), this, SLOT(updateVRoughness(double)));
  connect(btnAnisotropic,     SIGNAL(toggled(bool)),        this, SLOT(updateAnisotropic(bool)));
  connect(dispersion,         SIGNAL(toggled(bool)),        this, SLOT(updateDispersion(bool)));
  connect(cauchyB,            SIGNAL(valueChanged(double)), this, SLOT(updateCauchyB(double)));
  connect(clarity,            SIGNAL(valueChanged(double)), this, SLOT(updateClarity(double)));

  // The user selected one of the IOR presets
  connect(iorPresets, SIGNAL(itemSelectionChanged()), this, SLOT(selectIORPreset()));
  loadIORPresets();
  btnAnisotropic->setChecked(true);
  
}

QSize ReGlassEditor::sizeHint() const {
  return QSize(600, 340);
}


void ReGlassEditor::setData( ReMaterial* newMat,
                             ReTextureChannelDataModelPtr tcm ) 
{


  inSetup = true;
  currentMaterial = static_cast<Reality::ReGlass*>(newMat);
  textureChannelModel = tcm;
  
  switch(currentMaterial->getGlassType()) {
    case ReGlass::StandardGlass:
      standardGlass->setChecked(true);
      break;
    case ReGlass::VolumetricGlass:
      volumetricGlass->setChecked(true);
      break;
    case ReGlass::ArchitecturalGlass:
      architecturalGlass->setChecked(true);
      break;
    case ReGlass::FrostedGlass:
      frostedGlass->setChecked(true);
      break;
  }

  KrAdapter->setMaterial(newMat);
  KrAdapter->setTextureChannelModel(tcm);

  KtAdapter->setMaterial(newMat);
  KtAdapter->setTextureChannelModel(tcm);

  reflection->setTexture(currentMaterial->getKr(), currentMaterial);
  transmission->setTexture(currentMaterial->getKt(), currentMaterial);
  ior->setValue(currentMaterial->getIOR());
  presetName->setText(currentMaterial->getIORLabel());
  thinFilmIOR->setValue(currentMaterial->getThinFilmIOR());
  thinFilmThickness->setValue(currentMaterial->getThinFilmThickness());
  uRoughness->setValue(currentMaterial->getURoughness());  
  uRoughness->setValue(currentMaterial->getVRoughness());  
  dispersion->setChecked(currentMaterial->hasDispersion());
  cauchyB->setValue(currentMaterial->getCauchyB());
  adjustEditor();

  inSetup = false;
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));  

}



void ReGlassEditor::adjustEditor() {
  inSetup = true;
  if (standardGlass->isChecked() || architecturalGlass->isChecked()) {
    gbThinFilm->setEnabled(true);
  }
  else {
    gbThinFilm->setEnabled(false);
  }
  if (frostedGlass->isChecked()) {
    gbRoughness->setEnabled(true);
  }
  else {
    gbRoughness->setEnabled(false);
  }
  bool useDispersion = false;
  bool useCauchy     = false;
  bool useClarity    = false;
  if (standardGlass->isChecked()) {
    useDispersion = true;
    useCauchy     = true;
  }
  else if (volumetricGlass->isChecked()) {
    useDispersion = true;
    useCauchy     = true;
    useClarity    = true;
  }
  else if (frostedGlass->isChecked()) {
    useDispersion = true;
  }
  dispersion->setEnabled(useDispersion);
  cauchyB->setEnabled(useCauchy);
  clarity->setEnabled(useClarity);
  inSetup = false;
}

void ReGlassEditor::updateURoughness( double val ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setURoughness(val);
    if (btnAnisotropic->isChecked()) {
      vRoughness->setValue(val);      
    }    
    currentMaterial->setEdited();
    signalUpdate("uRoughness", val);    
  }
}

void ReGlassEditor::updateVRoughness( double val ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setVRoughness(val);
    currentMaterial->setEdited();
    signalUpdate("vRoughness", val);    
  }
}

void ReGlassEditor::updateAnisotropic( bool onOff ) {
  vRoughness->setEnabled(!onOff);
}

void ReGlassEditor::updateThinFilmThickness( double val ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setThinFilmThickness(val);
    currentMaterial->setEdited();
    signalUpdate("thinFilmThickness", val);    
  }
}

void ReGlassEditor::updateThinFilmIOR( double val ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setThinFilmIOR(val);
    currentMaterial->setEdited();
    signalUpdate("thinFilmIOR", val);    
  }  
}

void ReGlassEditor::createVolume() {
  // If the volume is already assigned it means that we created it before
  if (currentMaterial->getInnerVolume() != "") {
    return;
  }
  QString volName = QString("%1_g2vol").arg(currentMaterial->getUniqueName());
  ReVolumePtr vol = RealitySceneData->getVolume(volName);
  if (vol.isNull()) {
    vol = ReVolumePtr( new ReVolume(volName) );
    vol->setIOR(currentMaterial->getIOR());
    vol->setColor(currentMaterial->getKt()->getNamedValue("color").value<QColor>());
    vol->setClarityAtDepth(RE_GLASS_DEFAULT_CLARITY);
    if (currentMaterial->hasDispersion()) {
      vol->setCauchyB(currentMaterial->getCauchyB());
    }
    RealitySceneData->saveVolume(vol);
    RealitySceneData->linkVolumeToMaterial(volName, 
                                           currentMaterial->getParent()->getInternalName(), 
                                           currentMaterial->getName());
    currentMaterial->setInnerVolume(volName);
    emit volumeAdded(volName);
    signalUpdate("innerVolume", volName);
  }
}


void ReGlassEditor::updateGlassType() {
  // Find out which widget fired up.
  // Since the glass types are radiobutton, every time the user switches
  // to a new type of glass it causes this method to be called twice:
  // one call is for the previous glass type to be toggled off and one 
  // call is for the glass type to be toggled on. We don't care about the
  // "off" event.
  auto source = qobject_cast<QRadioButton*>(QObject::sender());
  if (!source) {
    return;
  }
  if (!source->isChecked()) {
    return;
  }

  if (currentMaterial && !inSetup ) {
    // Default value
    ReGlass::GlassType val = ReGlass::StandardGlass;
    if (standardGlass->isChecked()) {
      val = ReGlass::StandardGlass;
      // dispersion->setEnabled(false);
    }
    else if (volumetricGlass->isChecked()) {
      val = ReGlass::VolumetricGlass;
      createVolume();
      ReVolumePtr vol = RealitySceneData->getVolume(currentMaterial->getInnerVolume());
      if (!vol.isNull()) {        
        clarity->setValue(vol->getClarityAtDepth());
        vol->setCauchyB(RE_GLASS_DEFAULT_CAUCHYB);
        emit volumeUpdated(vol->getName());        
      }
      dispersion->setChecked(true);
      // cauchyB->setEnabled(true);
      cauchyB->setValue(RE_GLASS_DEFAULT_CAUCHYB);
      // clarity->setEnabled(true);
    }
    else if (architecturalGlass->isChecked()) {
      val = ReGlass::ArchitecturalGlass;
    }
    else if (frostedGlass->isChecked()) {
      val = ReGlass::FrostedGlass;
    }
    adjustEditor();
    currentMaterial->setGlassType(val);
    currentMaterial->setEdited();
    signalUpdate("glassType", val);    
  }  
}

void ReGlassEditor::updateIOR( double val ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setIOR(val);
    currentMaterial->setEdited();
    if (currentMaterial->getGlassType() == ReGlass::VolumetricGlass) {
      ReVolumePtr vol = RealitySceneData->getVolume(currentMaterial->getInnerVolume());
      if (!vol.isNull()) {
        vol->setIOR(val);
        emit volumeUpdated(vol->getName());
      }
    }
    signalUpdate("IOR", val);
    signalUpdate("IORLabel", presetName->text());
  }  
}


void ReGlassEditor::updateCauchyB( double val ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setCauchyB(val);
    if (currentMaterial->getGlassType() == ReGlass::VolumetricGlass) {
      ReVolumePtr vol = RealitySceneData->getVolume(currentMaterial->getInnerVolume());
      if (!vol.isNull()) {
        vol->setCauchyB(val);
        emit volumeUpdated(vol->getName());
      }
    }
    signalUpdate("cauchyB", val);
  }
}

void ReGlassEditor::updateClarity( double val ) {
  if (currentMaterial && !inSetup ) {
    if (currentMaterial->getGlassType() == ReGlass::VolumetricGlass) {
      ReVolumePtr vol = RealitySceneData->getVolume(currentMaterial->getInnerVolume());
      if (!vol.isNull()) {
        vol->setClarityAtDepth(val);
        emit volumeUpdated(vol->getName());
      }
    }
  }
}


void ReGlassEditor::updateDispersion( bool isOn ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setDispersion(isOn);
    if (currentMaterial->getGlassType() == ReGlass::VolumetricGlass) {
      ReVolumePtr vol = RealitySceneData->getVolume(currentMaterial->getInnerVolume());
      if (!vol.isNull()) {
        if (isOn) {
          vol->setCauchyB(cauchyB->value());
        }
        else {
          vol->setCauchyB(0.0);
        }
      }
    }
    signalUpdate("dispersion", isOn);
  }
}


void ReGlassEditor::selectIORPreset() {
  QList<QTreeWidgetItem*> selection = iorPresets->selectedItems();
  QString iorStr = selection[0]->text(1);
  // Clicked on a top-level node, no preset
  if (iorStr == "") {
    return;
  }
  // Show the preset's label
  presetName->setText(selection[0]->text(0));
  currentMaterial->setIORLabel(presetName->text());

  ior->setValue(iorStr.toDouble());
  signalUpdate("IORLabel", presetName->text());
}

void ReGlassEditor::loadIORPresets() {
#ifndef QT_CUSTOM_WIDGET

  QJson::Parser json;

  QFile colorPresetsRes(":/textResources/iorPresets.json");
  colorPresetsRes.open(QIODevice::ReadOnly);
  QByteArray data = colorPresetsRes.readAll();  
  colorPresetsRes.close();

  bool ok;
  QVariantMap presets = json.parse(data,&ok).toMap();
  if (!ok) {
    RE_LOG_DEBUG() << "Error: could not open the IOR preset resource.";
    return;
  }

  // The Map is grouped by names of major sets, like Glass, Liquids etc.

  QMapIterator<QString, QVariant> i(presets);
  QList<QTreeWidgetItem* > items;
  while( i.hasNext() ) {
    i.next();

    // Create the new top level group
    QTreeWidgetItem* newItem = new QTreeWidgetItem((QTreeWidgetItem*)0, QStringList(i.key()));

    QMapIterator<QString, QVariant> l(i.value().toMap());
    while( l.hasNext() ) {
      l.next();
      QTreeWidgetItem* iorPreset = new QTreeWidgetItem(newItem);
      iorPreset->setText(0, l.key());
      iorPreset->setText(1, l.value().toString());
      newItem->addChild( iorPreset );
    }
    // Add the new top-level item, the IOR preset group
    items.append( newItem );

  }
  // Add all the presets
  iorPresets->insertTopLevelItems(0, items);
#endif  
};

void ReGlassEditor::signalUpdate(QString valueName, QVariant value) {
  emit materialUpdated( currentMaterial->getParent()->getInternalName(), 
                        currentMaterial->getName(),
                        valueName,
                        value );
}



