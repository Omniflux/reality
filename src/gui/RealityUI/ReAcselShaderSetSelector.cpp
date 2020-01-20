/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include <QMessageBox>
#include "ReAcselShaderSetSelector.h"
#include "ReLogger.h"
#include "ReAcsel.h"

using namespace Reality;

ReAcselShaderSetSelector::ReAcselShaderSetSelector( QWidget* parent ) : 
  QDialog(parent)
{
  setupUi(this);

  auto acsel = ReAcsel::getInstance();
  QList<QStringList> shaderSets;
  acsel->getShaderSetList(shaderSets);

  int numRecs = shaderSets.count();
  shaderSetList->clear();
  for (int i = 0; i < numRecs; i++) {
    auto itm = new QTreeWidgetItem(0);
    // Set name
    itm->setText(0, shaderSets.at(i)[1]);
    // Description
    itm->setText(1, shaderSets.at(i)[2]);
    // Object ID
    itm->setText(2, shaderSets.at(i)[5]);
    // Is default?
    itm->setText(3, (shaderSets.at(i)[4] == "y" ? "Yes" : ""));
    // Store the set ID
    QString setID = shaderSets.at(i)[0];
    itm->setData(0, Qt::UserRole, setID);

    QList<QStringList> shaders;
    acsel->getShaderSetShaderInfo( setID, shaders );
    int numShaders = shaders.count();
    for (int l = 0; l < numShaders; l++) {
      auto shaderItem = new QTreeWidgetItem(itm);
      shaderItem->setText(0, shaders.at(l)[1]);
    }

    shaderSetList->addTopLevelItem(itm);
  }
  shaderSetList->resizeColumnToContents(0);
  shaderSetList->setColumnWidth(1, 200);
  shaderSetList->resizeColumnToContents(2);
  shaderSetList->resizeColumnToContents(3);
  shaderSetList->sortItems(0, Qt::AscendingOrder);

  currentShaderSet = -1;

  connect(shaderSetList, 
          SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
          this, SLOT(setSelectedShaderSet(QTreeWidgetItem*,QTreeWidgetItem*)));
}

void ReAcselShaderSetSelector::setSelectedShaderSet(QTreeWidgetItem* current,
                                                    QTreeWidgetItem* previous)
{
  if (current) {
    currentShaderSet = current->data(0, Qt::UserRole).toString();
  }
}

