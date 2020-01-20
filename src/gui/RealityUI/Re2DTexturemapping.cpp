/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "Re2DTextureMapping.h"

Re2DTextureMapping::Re2DTextureMapping(QWidget* parent) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  gain->setKeyboardTracking(false);
  gamma->setKeyboardTracking(false);
  uTile->setKeyboardTracking(false);
  vTile->setKeyboardTracking(false);
  uOffset->setKeyboardTracking(false);
  vOffset->setKeyboardTracking(false);
  connect(gain, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(gamma, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(uOffset, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(vOffset, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(uTile, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(vTile, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
  connect(texMapping, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValue(int)));
  inUpdate = false;
}

QSize Re2DTextureMapping::sizeHint() const {
  return QSize(245,171);
}

void Re2DTextureMapping::setModel( ReTextureChannelDataModelPtr newModel ) {
  model = newModel;
  inUpdate = true;
  gain->setValue(model->getNamedValue("gain").toFloat());
  gamma->setValue(model->getNamedValue("gamma").toFloat());
  uOffset->setValue(model->getNamedValue("uOffset").toFloat());
  vOffset->setValue(model->getNamedValue("vOffset").toFloat());
  uTile->setValue(model->getNamedValue("uTile").toFloat());
  vTile->setValue(model->getNamedValue("vTile").toFloat());
  texMapping->setCurrentIndex(model->getNamedValue("mapping").toInt());
  inUpdate = false;
}

void Re2DTextureMapping::updateValue( double newVal ) {
  if (inUpdate) {
    return;
  }
  QString source = QObject::sender()->objectName();

  if ( source == "gain" ) {
    model->setNamedValue("gain", newVal);
  }
  else if ( source == "gamma" ) {
    model->setNamedValue("gamma", newVal);    
  }
  else if ( source == "uTile" ) {
    model->setNamedValue("uTile", newVal);    
  }
  else if ( source == "vTile" ) {
    model->setNamedValue("vTile", newVal);    
  }
  else if ( source == "uOffset" ) {
    model->setNamedValue("uOffset", newVal);    
  }
  else if ( source == "vOffset" ) {
    model->setNamedValue("vOffset", newVal);
  }
  emit mappingChanged();
}

void Re2DTextureMapping::updateValue( int newVal ) {
  model->setNamedValue("mapping", static_cast<Re2DTexture::Mapping2D>(newVal));
}
