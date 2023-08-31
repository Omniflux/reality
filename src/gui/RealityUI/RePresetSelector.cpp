/*
 *  RePresetSelector.cpp
 *  ReGeomExport
 *
 *  Created by Paolo H Ciccone on 8/5/11.
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 *
 */

#include "RealityUI/RePresetSelector.h"

#include <QFile>
#include <QJson/Parser>

#include "ReLogger.h"


RePresetSelector::RePresetSelector( QWidget* parent ) : QDialog(parent) {
  setupUi(this);
  model.setColumnCount(2);
  QStringList headers; 
  headers << "Name" << "Description";
  model.setHorizontalHeaderLabels(headers);
  selectedValue = "<null>";
  btnOK->setEnabled(false);
  connect(tvPresets, SIGNAL(clicked(QModelIndex)), this, SLOT(showDescription(QModelIndex)));    
};


void RePresetSelector::setData( const QString presetFileName ) {
  QFile dataRes(presetFileName);
  QString dataStr;
  if (dataRes.open(QIODevice::ReadOnly)) {
    dataStr = dataRes.readAll();
    dataRes.close();
  }
  else {
    RE_LOG_DEBUG() << "Error: cannot open the resource " << QSS(presetFileName);
    return;
  }

  QJson::Parser jsonParser;
  bool ok;

  QVariantList presets = jsonParser.parse(dataStr.toUtf8(), &ok).toList();
  if (!ok) {
    RE_LOG_DEBUG() << "Error: syntax error in JSON file " << QSS(presetFileName);
    return;
  }

  unsigned int numItems = presets.count();
  QStandardItem *parentItem = model.invisibleRootItem();
  for (unsigned i=0; i<numItems; i++) {
    // Get the nth item in the list
    QVariantMap listItem = presets.at(i).toMap();
    // menuText has the title for this item
    QString key = listItem["menuText"].toString(); 
    QStandardItem *item = new QStandardItem(key);
    item->setSelectable(false);
    item->setEditable(false);
    parentItem->appendRow(item);

    QVariantList lightData = listItem["options"].toList();
    int numLights = lightData.count();
    for (int l = 0; l < numLights; ++l) {
      QVariantMap lightPreset = lightData.at(l).toMap();
      QStandardItem* nameItem = new QStandardItem(lightPreset["name"].toString());
      QStandardItem* descItem = new QStandardItem(lightPreset["desc"].toString());
      nameItem->setEditable(false);
      item->setChild(l,0,nameItem);
      item->setChild(l,1,descItem);
      nameItem->setData(lightPreset["value"].toString());
    }
  }
  tvPresets->setModel(&model);
  tvPresets->setColumnWidth(0, 250);
  tvPresets->expandAll();
}

void RePresetSelector::showDescription(const QModelIndex& index) {
  QStandardItem *item, *descItem;    
  // If the user clicked on the description column we re-create an index
  // that simulates a click on the name column
  item = model.itemFromIndex(model.index(index.row(), 0, index.parent()));
  descItem = model.itemFromIndex(model.index(index.row(), 1, index.parent()));
  selectedValue = item->data().toString();
  selectedText  = item->text();
  selectedDescription = descItem->text();
  lblDescription->setText(descItem->text());
  btnOK->setEnabled(!selectedDescription.isEmpty());
};


/**
 * Returns the value selected in case the user pressed the "OK" button in the dialog box
 */
QString RePresetSelector::getValue() {
  return(selectedValue);
}
/**
 * Returns the text of the selected item. This is the text in the first colum, what is identified as "name"
 */
QString RePresetSelector::getText() {
  return(selectedText);
}

QString RePresetSelector::getDescription() {
  return(selectedDescription);
}
