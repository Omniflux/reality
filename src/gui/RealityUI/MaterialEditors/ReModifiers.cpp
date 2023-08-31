/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReDisplaceableMaterial.h"
#include "ReModifiers.h"
#include "ReTools.h"


ReModifiers::ReModifiers( QWidget* parent ) : 
  ReMaterialEditor(parent) 
{
  setupUi(this);
  inSetup = false;
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

  // Avoid getting a "modified" event every time a character is changed in the fields
  bmStrength->setKeyboardTracking(false);
  bmNegative->setKeyboardTracking(false);
  bmPositive->setKeyboardTracking(false);

  dmStrength->setKeyboardTracking(false);
  dmNegative->setKeyboardTracking(false);
  dmPositive->setKeyboardTracking(false);
  lightGain->setKeyboardTracking(false);

  bumpMap->setAllowedDataTypes(RE_TEX_DATATYPE_NUMERIC);
  displacementMap->setAllowedDataTypes(RE_TEX_DATATYPE_NUMERIC);
  ambientTex->setAllowedDataTypes(RE_TEX_DATATYPE_COLOR);

  connect(smoothSurface, SIGNAL(toggled(bool)), this, SLOT(setSmoothness(bool)));

  bmAdapter = ReTextureEditorAdapterPtr( 
                new ReTextureEditorAdapter(
                      this, 
                      bumpMap, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::numeric,
                      textureChannelModel
                    )
              );
  dmAdapter = ReTextureEditorAdapterPtr( 
                new ReTextureEditorAdapter(
                      this, 
                      displacementMap, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::numeric,
                      textureChannelModel
                    )
              );
  klAdapter = ReTextureEditorAdapterPtr( 
                new ReTextureEditorAdapter(
                      this, 
                      ambientTex, 
                      currentMaterial, 
                      ReTexture::ReTextureDataType::color,
                      textureChannelModel
                    )
              );

  connect(bmAdapter.data(), SIGNAL(textureModified()), SLOT(refreshUI()));
  connect(dmAdapter.data(), SIGNAL(textureModified()), SLOT(refreshUI()));
  connect(klAdapter.data(), SIGNAL(textureModified()), SLOT(refreshUI()));

  connect(bmStrength, SIGNAL(valueChanged(int)), this, SLOT(updateBmStrength(int)));
  connect(bmNegative, SIGNAL(valueChanged(double)), this, SLOT(updateBmNegative(double)));
  connect(bmPositive, SIGNAL(valueChanged(double)), this, SLOT(updateBmPositive(double)));

  connect(dmStrength, SIGNAL(valueChanged(int)), this, SLOT(updateDmStrength(int)));
  connect(dmNegative, SIGNAL(valueChanged(double)), this, SLOT(updateDmNegative(double)));
  connect(dmPositive, SIGNAL(valueChanged(double)), this, SLOT(updateDmPositive(double)));

  connect(subdivisions, SIGNAL(valueChanged(int)), this, SLOT(updateSubdivisions(int)));
  connect(keepSharpEdges, SIGNAL(toggled(bool)), this, SLOT(updateKeepSharpEdges(bool)));
  connect(useMicroFacets, SIGNAL(toggled(bool)), this, SLOT(updateUseMicrofacets(bool)));
  // Light emission
  connect(hasLight, SIGNAL(toggled(bool)), this, SLOT(updateHasLight(bool)));
  connect(lightGain, SIGNAL(valueChanged(double)), this, SLOT(updateLightGain(double)));
}

QSize ReModifiers::sizeHint() const {
  return QSize(680, 335);
}

void ReModifiers::refreshUI() {
  inSetup = true;
  ReModifiedMaterial* material = static_cast<ReModifiedMaterial*>(currentMaterial);

  bumpMap->setTexture(material->getBumpMap(), material);
  bumpMap->setDeleteEnabled(true);
  bmStrength->setValue(material->getBmStrength() * 100.0);
  bmPositive->setValue(material->getBmPositive());
  bmNegative->setValue(material->getBmNegative());
  ReTexturePtr bm = material->getChannel("Bm");
  // Disable the BM controls if we have a Normal Map
  if (!bm.isNull()) {
    // if (bm->getType() == TexImageMap && bm->getNamedValue("normalMap").toBool()) {
    if (containsNormalMap(bm)) {
      bmNegative->setEnabled(false);
      bmPositive->setEnabled(false);
    }
    else {
      bmNegative->setEnabled(true);
      bmPositive->setEnabled(true);
    }
  }
  // Displacement
  displacementMap->setTexture(material->getDisplacementMap(), material);
  displacementMap->setDeleteEnabled(true);
  dmStrength->setValue(material->getDmStrength()*100.0);
  dmPositive->setValue(material->getDmPositive());
  dmNegative->setValue(material->getDmNegative());

  // Ambient
  ambientTex->setTexture(material->getAmbientMap(), material);
  ambientTex->setDeleteEnabled(true);
  
  subdivisions->setValue(material->getSubdivisions());
  keepSharpEdges->setChecked(material->keepsSharpEdges());
  useMicroFacets->setChecked(material->usesMicrofacets());
  smoothSurface->setChecked(material->isSmooth());  

  // Light emission
  hasLight->setChecked(material->isEmittingLight());
  lightGain->setValue(material->getLightGain());

  inSetup = false;
  initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));
}

void ReModifiers::setData( ReMaterial* newMat, 
                           ReTextureChannelDataModelPtr tcm ) 
{
  currentMaterial = newMat; //static_cast<ReModifiedMaterial*>(newMat);
  textureChannelModel = tcm;

  bmAdapter->setMaterial(newMat);
  bmAdapter->setTextureChannelModel(tcm);

  dmAdapter->setMaterial(newMat);
  dmAdapter->setTextureChannelModel(tcm);

  klAdapter->setMaterial(newMat);
  klAdapter->setTextureChannelModel(tcm);

  refreshUI();
}

void ReModifiers::updateBmStrength( int newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    float fv = newVal / 100.0;
    currentMaterial->setNamedValue("bmStrength", fv);
    signalUpdate("bmStrength", fv);
  }
}

void ReModifiers::updateBmNegative( double newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("bmNegative", newVal);
    signalUpdate("bmNegative", newVal);
  }

}

void ReModifiers::updateBmPositive( double newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("bmPositive",newVal);
    signalUpdate("bmPositive", newVal);
  }

}

void ReModifiers::updateDmStrength( int newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    float fv = newVal / 100.0;    
    currentMaterial->setNamedValue("dmStrength",fv);
    signalUpdate("dmStrength", fv);
  }
}

void ReModifiers::updateDmNegative( double newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("dmNegative",newVal);
    signalUpdate("dmNegative", newVal);
  }

}

void ReModifiers::updateDmPositive( double newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("dmPositive",newVal);
    signalUpdate("dmPositive", newVal);
  }

}

void ReModifiers::updateSubdivisions( int newVal ) {
  if (!inSetup) {
    //  Cannot have 0 subdivisions if a displacement map is present.
    ReTexturePtr dm = currentMaterial->getNamedValue("displacementMap")
                        .value<ReTexturePtr>();
    if ( newVal == 0 && !dm.isNull() ) 
    {
      newVal = 1;
      subdivisions->setValue(newVal);
    }
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("subdivision",newVal);
    signalUpdate("subdivision", newVal);
  }
}

void ReModifiers::updateKeepSharpEdges( bool newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("keepSharpEdges",newVal);
    signalUpdate("keepSharpEdges", newVal);
  }
}

void ReModifiers::updateUseMicrofacets( bool newVal ) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("useMicrofacetsFlag",newVal);
    // The keep sharp edges flag is used only by loop subdivision.
    keepSharpEdges->setEnabled(!useMicroFacets->isChecked());
    signalUpdate("useMicrofacetsFlag", newVal);
  }
}

void ReModifiers::updateHasLight(bool newVal) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("emitsLight", newVal);
    signalUpdate("emitsLight", newVal);
  }  
};

void ReModifiers::updateLightGain(double newVal) {
  if (!inSetup) {
    currentMaterial->setEdited();
    currentMaterial->setNamedValue("lightGain", newVal);
    signalUpdate("lightGain", newVal);
  }  
};

void ReModifiers::setSmoothness( bool checked ) {
  currentMaterial->setNamedValue("smoothnessFlag", checked);
}

void ReModifiers::adjustSubdivision() {
  if ( subdivisions->value() == 0 && displacementMap->hasTexture() ) {
    subdivisions->setValue(RE_MIN_LOOP_SUBD);
  }
}
