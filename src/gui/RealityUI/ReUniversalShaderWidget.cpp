/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "RealityUI/ReUniversalShaderWidget.h"

#include "ReAcsel.h"
#include "RealityUI/ReShaderCategoryPreset.h"


using namespace Reality;

ReUniversalShaderWidget::ReUniversalShaderWidget( QWidget* parent ) : 
  QWidget(parent)
{
  setupUi(this);
  // Initialize the combobox with the list of categories
  categoryPreset = ReShaderCategoryPresetPtr( 
                     new ReShaderCategoryPreset(false) 
                   );
  categoryPreset->initComboBox(categories);

  connect(shaderList, 
          SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
          this, SLOT(setSelectedShaderSet(QTreeWidgetItem*,QTreeWidgetItem*)));
  connect(categories, SIGNAL(currentIndexChanged(const QString&)), 
          this, SLOT(updateShaderList(const QString&)));

  displayShaders();
}

void ReUniversalShaderWidget::displayShaders( const QString& category ) {
  updatingUI = true;
  ShaderCollection shaders;
  auto acsel = ReAcsel::getInstance();
  QString defaultCat = category;
  if ( category.isEmpty() ) {
    defaultCat = tr("Metal");
  }
  categories->setCurrentIndex(categoryPreset->getID(defaultCat));

  acsel->getUniversalShaderList(shaders, defaultCat);
  int numRecs = shaders.count();
  shaderList->clear();
  for (int i = 0; i < numRecs; i++) {
    auto itm = new QTreeWidgetItem(0);
    // Name
    itm->setText(0, shaders.at(i)[1]);
    // Description
    itm->setText(1, shaders.at(i)[2]);
    // Store the set ID
    itm->setData(0, Qt::UserRole, shaders.at(i)[0]);

    shaderList->addTopLevelItem(itm);
  }
  shaderList->resizeColumnToContents(0);
  shaderList->setColumnWidth(1, 200);
  shaderList->resizeColumnToContents(2);
  shaderList->sortItems(0, Qt::AscendingOrder);

  currentShader = "";
  updatingUI = false;  
}

QStringList ReUniversalShaderWidget::getSelectedShaderIDs() {
  auto rows = shaderList->selectedItems();
  int numRows = rows.count();
  QStringList IDs;
  for (int i = 0; i < numRows; i++) {
    IDs << rows[i]->data(0, Qt::UserRole).toString();
  }
  return IDs;
}

void ReUniversalShaderWidget::setSelectedShaderSet( QTreeWidgetItem* current,
                                                    QTreeWidgetItem* previous)
{
  if (current) {
    currentShader = current->data(0, Qt::UserRole).toString();
    unsigned char* bitmapData;
    int blobSize = 0;
    ReAcsel::getInstance()->getUniversalShaderPreview(currentShader, 
                                                      bitmapData,
                                                      blobSize);
    if (blobSize > 0) {
      QPixmap p;
      p.loadFromData(bitmapData, blobSize);
      preview->setPixmap(p);
      delete bitmapData;
    }
    else {
      preview->clear();      
    }
    emit shaderSelected(currentShader);
  }
}

void ReUniversalShaderWidget::updateShaderList( const QString& newCat ) {
  if (!updatingUI) {
    displayShaders(newCat);
  }
}

