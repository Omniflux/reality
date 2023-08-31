/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReColorTextureEditor.h"

#include <QVariant>
#include <QJson/Parser>

#include "ReLogger.h"


void ReColorTextureEditor::initColorPresets() {
  QJson::Parser json;

  QFile colorPresetsRes(":/textResources/colorPresets.json");
  colorPresetsRes.open(QIODevice::ReadOnly);
  QByteArray data = colorPresetsRes.readAll();  
  colorPresetsRes.close();

  bool ok;
  QVariantMap presets = json.parse(data,&ok).toMap();
  if (!ok) {
    RE_LOG_DEBUG() << "Error: could not open the color preset resource.";
    return;
  }
  QMapIterator<QString, QVariant> i(presets);
  while(i.hasNext()) {
    i.next();
    QString groupName = i.key();
    // A color group is a list of 5 arrays. 
    QVariantList colorGroups = i.value().toList();
    int numColorGroups = colorGroups.count();

    QList<ReColorList> aColorGroup;
    for (int i = 0; i < numColorGroups; ++i) {
      QVariantList colorGroup = colorGroups[i].toList();
      int numColors = colorGroup.count();
      // Read each color from a single group/row. There are
      // 5 colors per row. Each colro is represented by an
      // array where [0]=r, [1]=g, [2]=b
      ReColorList aColorRow;
      for (int l = 0; l < numColors; ++l) {
        QVariantList aColor = colorGroup[l].toList();
        aColorRow.append(QColor(aColor[0].toInt(), aColor[1].toInt(), aColor[2].toInt()));
      }
      aColorGroup.append(aColorRow);
    }
    colorPresets[groupName] = aColorGroup;
  }
  updateColorPreset(0);
}


void ReColorTextureEditor::loadPreset( const QString presetName ) {
  QListIterator<ReColorList> i(colorPresets[presetName]);

  int rowNo=0;
  while(i.hasNext()) {
    ReColorList aRow = i.next();
    int numColors = aRow.count();
    for (int l = 0; l < numColors; ++l) {
      QColor clr = aRow[l];
      QLabel* swatch = findChild<QLabel*>(QString("clrPreset%1_%2").arg(rowNo+1).arg(l+1));
      if (swatch) {
        swatch->setStyleSheet(QString("background-color: %1;").arg(clr.name()));
        swatch->setProperty("colorValue", clr.rgba());
      }
    }
    rowNo++;
  }
}

void ReColorTextureEditor::updateColorPreset( int index ) {
  QString presetName = presetSelector->itemText(index);

  if (presetName == tr("Hair")) {
    loadPreset("hair");
  }
  else if (presetName == tr("Skin")) {
    loadPreset("skin");
  }
  else if (presetName == tr("Plastic")) {
    loadPreset("plastic");
  }
  else if (presetName == tr("Wood")) {
    loadPreset("wood");
  }
}

void ReColorTextureEditor::setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
  model = _model;
  material = mat;
  connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  refreshUI();
}

void ReColorTextureEditor::refreshUI() {
  if (updating) {
    return;
  }
  teColorPicker->setColor(model->getNamedValue("color").value<QColor>());
}

bool ReColorTextureEditor::eventFilter(QObject* obj, QEvent* event) {
  if (event->type() == QEvent::MouseButtonRelease) {
    teColorPicker->setColor(QColor(obj->property("colorValue").toInt()));
    emit updateTexture();
    return true;
  }
  return false;
}
