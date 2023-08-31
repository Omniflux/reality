/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReSkinEditor.h"

#include "ReMaterialPreview.h"
#include "ReSkin.h"
#include "exporters/lux/ReSkinExporter.h"



#define RE_SKIN_HM_PREVIEW_ID "hairmask"

ReSkinEditor::ReSkinEditor( QWidget* parent ) :
  ReMaterialEditor(parent)
{
  setupUi(this);
  inSetup = false;
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

  previewMaker = NULL;
  // Avoid getting a "modified" event every time a character is changed in the fields
  uGlossiness->setKeyboardTracking(false);
  vGlossiness->setKeyboardTracking(false);
  clarityAtDepth->setKeyboardTracking(false);
  scatteringScale->setKeyboardTracking(false);
  absorptionScale->setKeyboardTracking(false);
  fresnel->setKeyboardTracking(false);
  hmGain->setKeyboardTracking(false);
  coatThickness->setKeyboardTracking(false);

  // Enable the Delete menu for the Kg texture
  skinKg->setDeleteEnabled(true);

  // Enable the Delete menu for the transclucency texture
  skinKa->setDeleteEnabled(true);

  // Make sure that there is the delete option for the Hair Mask
  Khm->setDeleteEnabled(true);

  KdAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      skinKd, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KsAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      skinKs, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KgAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      skinKg, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KhmAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      Khm, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  KaAdapter = QSharedPointer<ReTextureEditorAdapter>( 
                new ReTextureEditorAdapter(
                      this, 
                      skinKa, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );
  // Responds to the texture modified signal by refreshin the hair mask preview
  connect(KhmAdapter.data(), SIGNAL(textureModified()), 
          this, SLOT(refreshHairMaskPreview()));

  connect(surfaceColor,  SIGNAL(colorChanged()), this, SLOT(updateSurfaceColor()));
  connect(interiorColor, SIGNAL(colorChanged()), this, SLOT(updateInteriorColor()));

  connect(fresnel , SIGNAL(valueChanged(int)), this, SLOT(updateFresnel(int)));

  connect(uGlossiness, SIGNAL(valueChanged(int)), this, SLOT(updateUGlossiness(int)));
  connect(vGlossiness, SIGNAL(valueChanged(int)), this, SLOT(updateVGlossiness(int)));
  connect(surfaceFuzz, SIGNAL(toggled(bool)),     this, SLOT(updateSurfaceFuzz(bool)));
  connect(hairMask,    SIGNAL(toggled(bool)),     this, SLOT(updateHairMask(bool)));
  connect(SSS,         SIGNAL(toggled(bool)),     this, SLOT(updateSSS(bool)));
  connect(gbCoat,      SIGNAL(toggled(bool)),     this, SLOT(updateCoat(bool)));
  connect(coatThickness, SIGNAL(valueChanged(double)),     this, SLOT(updateCoatThickness(double)));
  connect(clarityAtDepth, SIGNAL(valueChanged(double)), this, SLOT(updateClarityAtDepth(double)));
  connect(scatteringScale, SIGNAL(valueChanged(double)), this, SLOT(updateScatteringScale(double)));
  connect(absorptionScale, SIGNAL(valueChanged(double)), this, SLOT(updateAbsorptionScale(double)));
  connect(btnAnisotropic,  SIGNAL(toggled(bool)),        this, SLOT(updateAnisotropic(bool)));
  connect(hmGain,  SIGNAL(valueChanged(double)),        this, SLOT(updateHmGain(double)));

  // Kt must be a color texture
  skinKa->setAllowedDataTypes(RE_TEX_DATATYPE_COLOR);

  connectToPreviewMaker();  
}

QSize ReSkinEditor::sizeHint() const {
  return QSize(672, 300);
}

void ReSkinEditor::setData( ReMaterial* newMat, 
                            ReTextureChannelDataModelPtr tcm ) 
{

  inSetup = true;
  currentMaterial = newMat;
  textureChannelModel = tcm;
  ReSkin* mat = static_cast<ReSkin*>(currentMaterial);
  skinKd->setTexture(mat->getKd(), mat);
  skinKs->setTexture(mat->getKs(), mat);
  skinKa->setTexture(mat->getKa(), mat);

  updateSSSVolumeInfo();
  
  uGlossiness->setValue(mat->getUGlossiness());
  vGlossiness->setValue(mat->getVGlossiness());
  btnAnisotropic->setChecked(mat->getUGlossiness() == mat->getVGlossiness());
  updateAnisotropic(btnAnisotropic->isChecked());

  skinKg->setTexture(mat->getKg(), mat);
  surfaceFuzz->setChecked(mat->getSurfaceFuzz());

  fresnel->setValue(mat->getFresnelAmount() * 100.0f);
  // We express this value here in mm instead of mt
  clarityAtDepth->setValue(mat->getNamedValue("clarityAtDepth").toFloat()*1000.0);
  scatteringScale->setValue(mat->getNamedValue("scatteringScale").toFloat());
  absorptionScale->setValue(mat->getNamedValue("absorptionScale").toFloat());
  SSS->setChecked(mat->isSSSEnabled());
  bool hasHairMask = mat->hasHairMask();
  hairMask->setChecked(hasHairMask);
  hmGain->setValue(mat->getHmGain());
  if (hasHairMask) {
    Khm->setTexture(mat->getKhm(), mat); 
    refreshHairMaskPreview();
  }
  else {
    Khm->setBlank();
    hmPreview->clear();
  }
  gbCoat->setChecked(mat->hasTopCoat());
  coatThickness->setValue(mat->getCoatThickness());
  
  inSetup = false;
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));

  KdAdapter->setMaterial(newMat);
  KdAdapter->setTextureChannelModel(tcm);

  KsAdapter->setMaterial(newMat);
  KsAdapter->setTextureChannelModel(tcm);

  KgAdapter->setMaterial(newMat);
  KgAdapter->setTextureChannelModel(tcm);

  KhmAdapter->setMaterial(newMat);
  KhmAdapter->setTextureChannelModel(tcm);

  KaAdapter->setMaterial(newMat);
  KaAdapter->setTextureChannelModel(tcm);
}

void ReSkinEditor::updateSSSVolumeInfo() {
  surfaceColor->setColor(currentMaterial->getNamedValue("absorptionColor").value<QColor>());
  interiorColor->setColor(currentMaterial->getNamedValue("scatteringColor").value<QColor>());  
}

void ReSkinEditor::connectToPreviewMaker() {
  // Retrieve the pointer to the material preview maker
  previewMaker = ReMaterialPreview::getInstance(); 
  if (previewMaker) {
    connect(previewMaker, SIGNAL(previewReady(QString, QString,QImage*)),
            this, SLOT(updatePreview(QString, QString,QImage*)));
  }  
}

void ReSkinEditor::updatePreview( QString matName, 
                                  QString previewID,
                                  QImage* preview ) 
{
  if (!(previewMaker && currentMaterial)) {
    return;
  }
  if (previewID == RE_SKIN_HM_PREVIEW_ID) {
    hmPreview->setPixmap(QPixmap::fromImage(*preview).scaled(90, 90));
    // We are responsible for the disposal of this resources
    delete preview;
  }
}

void ReSkinEditor::refreshHairMaskPreview() {
  if (previewMaker == NULL) {
    return;
  }

  // Get the material definition
  QString matDefinition;
  ReSkinLuxExporter::makeHairMaskTexture("hairmask",
                                         "TextureMonitor",
                                         static_cast<ReSkin*>(currentMaterial),
                                         matDefinition);

  matDefinition += QString(
                     "MakeNamedMaterial \"TextureMonitor\" \"string type\" [\"matte\"] "
                     "\"texture Kd\" [\"hairmask\"]\n"
                   );

  ReMaterialPreview::getInstance()->sendPreviewRequest(
    "TextureMonitor", 
    RE_SKIN_HM_PREVIEW_ID, 
    "hairmask", 
    matDefinition,
    false, 
    true
  );  
}

void ReSkinEditor::updateUGlossiness(int newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("uGlossiness", newVal);
    if (btnAnisotropic->isChecked()) {
      vGlossiness->setValue(newVal);      
    }
    signalUpdate("uGlossiness", newVal);
  }
} 

void ReSkinEditor::updateVGlossiness(int newVal) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("vGlossiness",newVal);
    currentMaterial->setEdited();
    signalUpdate("vGlossiness", newVal);
  }
} 

void ReSkinEditor::updateSurfaceFuzz( bool newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("surfaceFuzz",newVal);
    currentMaterial->setEdited();
    signalUpdate("surfaceFuzz", newVal);
  }
}

void ReSkinEditor::updateSSS( bool onOff ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("sssEnabled",onOff);
    currentMaterial->setEdited();
    signalUpdate("sssEnabled", onOff);
    updateSSSVolumeInfo();
    emit materialVolumeChanged(currentMaterial->getInnerVolume(), onOff);
  }
}

void ReSkinEditor::updateCoat( bool onOff ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("topCoat",onOff);
    currentMaterial->setEdited();
    signalUpdate("topCoat", onOff);
  }
}

void ReSkinEditor::updateCoatThickness( double thickness ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("coatThickness", thickness);
    currentMaterial->setEdited();
    signalUpdate("coatThickness", thickness);
  }
}

void ReSkinEditor::updateHairMask( bool newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("hairMask",newVal);
    currentMaterial->setEdited();
    if (newVal) {
      Khm->setTexture(currentMaterial->getNamedValue("Khm").value<ReTexturePtr>(), currentMaterial);
    }
    else {
      Khm->setBlank();
    }
    signalUpdate("hairMask", newVal);
  }
}

void ReSkinEditor::updateHmGain( double newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("hmGain",newVal);
    refreshHairMaskPreview();
    currentMaterial->setEdited();
    signalUpdate("hmGain", newVal);
  }
}


void ReSkinEditor::updateFresnel( int newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("fresnelAmount",newVal/100.0f);
    currentMaterial->setEdited();
    signalUpdate("fresnelAmount", currentMaterial->getNamedValue("fresnelAmount"));
  }
}

void ReSkinEditor::updateClarityAtDepth( double newVal ) {
  if (currentMaterial && !inSetup ) {
    // We use mm, the volumes are stored in mt
    newVal /= 1000;
    currentMaterial->setNamedValue("clarityAtDepth",newVal);
    currentMaterial->setEdited();
    signalUpdate("clarityAtDepth", newVal);
  }
}

void ReSkinEditor::updateScatteringScale( double newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("scatteringScale",newVal);
    currentMaterial->setEdited();
    signalUpdate("scatteringScale", newVal);
  }
}

void ReSkinEditor::updateAbsorptionScale( double newVal ) {
  if (currentMaterial && !inSetup ) {
    currentMaterial->setNamedValue("absorptionScale",newVal);
    currentMaterial->setEdited();
    signalUpdate("absorptionScale", newVal);
  }
}

void ReSkinEditor::updateAnisotropic( bool onOff ) {
  if (onOff) {
    vGlossiness->setValue(uGlossiness->value());
  }
  vGlossiness->setEnabled(!onOff);
}

void ReSkinEditor::updateInteriorColor() {
  if (currentMaterial && !inSetup ) {
    QColor col = interiorColor->getColor();
    currentMaterial->setNamedValue("scatteringColor", col);
    currentMaterial->setEdited();
    signalUpdate("scatteringColor", col);
  }  
}

void ReSkinEditor::updateSurfaceColor() {
  if (currentMaterial && !inSetup ) {
    QColor col = surfaceColor->getColor();
    currentMaterial->setNamedValue("absorptionColor", col);
    currentMaterial->setEdited();
    signalUpdate("absorptionColor", col);
  }  
}

