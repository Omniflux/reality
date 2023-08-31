/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReAcselMgr.h"

#include <QClipboard>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

#include "ReAcsel.h"
#include "RealityBase.h"
#include "RealityUI/ReUniversalShaderWidget.h"
#include "ui_reAcselOverwriteConfirm.h"


#define RE_ACSEL_MGR_CONFIG_KEY "AcselMgr"

using namespace Reality;

ReAcselManager::ReAcselManager( QWidget* parent ) : QDialog(parent) 
{
  setupUi(this);

  // Add the Universal Shader selector to the UI
  auto vLayout = new QVBoxLayout(selectorContainer);
  vLayout->setObjectName(QString::fromUtf8("selectorLayout"));
  usSelector = new ReUniversalShaderWidget(selectorContainer);
  vLayout->addWidget(usSelector);
  usSelector->setMessage("");
  usSelector->enableMultipleSelections(true);

  connect(btnACSELImport, SIGNAL(clicked()), this, SLOT(importBundle()));
  connect(btnACSELExport, SIGNAL(clicked()), this, SLOT(exportShaderSets()));
  connect(btnACSELDelete, SIGNAL(clicked()), this, SLOT(deleteShaderSet()));
  connect(btnACSELEdit, SIGNAL(clicked()), this, SLOT(editShaderSet()));

  connect(btnUniShaderExport, SIGNAL(clicked()), 
          this, SLOT(exportUniversalShaders()));
  connect(btnUniShaderImport, SIGNAL(clicked()),this, SLOT(importBundle()));
  connect(btnUniShaderDelete, SIGNAL(clicked()),
          this, SLOT(deleteUniversalShaders()));

  connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(switchTab(int)));
  connect(shaderList->selectionModel(), 
          SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection& )), 
          this, SLOT(handleSelection(const QItemSelection&, const QItemSelection&)));

  // Add the copy (Cmd-C) action to copy the list of selected shaders to
  // the clipboard
  auto copyAction = new QAction(this);
  copyAction->setShortcut(QKeySequence::Copy);
  shaderList->addAction(copyAction);
  connect( copyAction, SIGNAL(triggered()), this, SLOT(copyShaderList()));

  updateUI();
  editControls->setEnabled(false);
  urlLoaded = false;

  // Restore the window size and position
  ReConfigurationPtr settings = RealityBase::getConfiguration();
  settings->beginGroup(RE_ACSEL_MGR_CONFIG_KEY);
  resize(settings->value("size", size()).toSize());
  move(settings->value("pos", pos()).toPoint());
  settings->endGroup();
};

void ReAcselManager::loadAcselShaders() {
  auto acsel = ReAcsel::getInstance();
  QList<QStringList> records;
  acsel->getShaderSetList( records );
  int numRecs = records.count();
  shaderList->clear();
  for (int i = 0; i < numRecs; i++) {
    auto itm = new QTreeWidgetItem(0);
    // Set name
    itm->setText(0, records.at(i)[1]);
    // Description
    itm->setText(1, records.at(i)[2]);
    // Date created
    itm->setText(2, records.at(i)[3]);
    // Is default/generic?
    itm->setText(3, (records.at(i)[4] == "y" ? tr("Yes") : ""));
    // Is enabled?
    itm->setText(4, (records.at(i)[6] == "y" ? tr("Yes") : ""));
    // Author
    itm->setText(5, records.at(i)[7]);
    // Object ID
    itm->setText(6, records.at(i)[5]);
    // Store the set ID
    itm->setData(0, Qt::UserRole, records.at(i)[0]);
    shaderList->addTopLevelItem(itm);
  }
  shaderList->resizeColumnToContents(0);
  shaderList->setColumnWidth(1, 200);
  shaderList->resizeColumnToContents(2);
  shaderList->resizeColumnToContents(3);
  shaderList->sortItems(0, Qt::AscendingOrder);
  stats->setText(QString(tr("Found %1 presets")).arg(numRecs));
};

void ReAcselManager::updateUI() {
  loadAcselShaders();
  usSelector->update();
  // Hide the Community tab for now. We don't have the time to
  // make it work for 4.0
  tabs->removeTab(2);
}


void ReAcselManager::deleteShaderSet() {
  if (QMessageBox::question(
        this, 
        tr("Deletion confirmation"), 
        tr("Are you sure that you want to delete the selected "
           "shader sets?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        ) != QMessageBox::Yes )
  {
    return;
  }
  // Find which shader sets have been selected
  auto items = shaderList->selectedItems();
  int count = items.count();
  auto acsel = ReAcsel::getInstance();
  for (int i = 0; i < count; i++) {    
    acsel->deleteShaderSet(items[i]->data(0, Qt::UserRole).toString());
  }  
  updateUI();
};


void ReAcselManager::deleteUniversalShaders() {
  if (QMessageBox::question(
        this, 
        tr("Deletion confirmation"), 
        tr("Are you sure that you want to delete the selected "
           "universal shaders?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        ) != QMessageBox::Yes )
  {
    return;
  }

  QString category = usSelector->getSelectedCategory();
  // Find which shaders have been selected
  auto shaderIDs = usSelector->getSelectedShaderIDs();
  int count = shaderIDs.count();
  auto acsel = ReAcsel::getInstance();
  for (int i = 0; i < count; i++) {    
    if ( !acsel->deleteUniversalShader(shaderIDs[i]) ) {
      QMessageBox::information(
        this, 
        tr("Error deliting shader"), 
        tr("Shader %1 could not be deleted")
      );
    }
  }  
  usSelector->displayShaders(category);
}


void ReAcselManager::importBundle() {
  QString fileName = QFileDialog::getOpenFileName(
    this,
    tr("Select the ACSEL Bundle to import"),
    "",
    tr("ACSEL Bundle (*.rsb)")
  );
  if (fileName.isEmpty()) {
    return;
  }
  auto acsel = ReAcsel::getInstance();
  ReAcsel::ExistingShaderSets existingShaderSets;
  ReAcsel::ReturnCode retval = acsel->importBundle(
                                 fileName, 
                                 false, 
                                 existingShaderSets
                               );
  switch(retval) {
    case ReAcsel::ShaderSetAlreadyExists: {
      QDialog confirm(this);
      Ui::AcselOverwriteConfirm ui;
      ui.setupUi(&confirm);
      QMapIterator<QString, QString> i(existingShaderSets);
      while ( i.hasNext() ) {
        i.next();
        auto itm = new QTreeWidgetItem(0);
        itm->setText(0, i.key());
        itm->setText(1, i.value());
        ui.shaderList->addTopLevelItem(itm);
      }      
      if (confirm.exec() == QDialog::Rejected) {
        QMessageBox::information(
          this,
          tr("Information"),
          tr("The shader sets have not been changed")
        );
        return;
      }
      // User confirmed the overwriting so we re-execute the import, this
      // time with the overwrite flag
      acsel->importBundle(fileName, true, existingShaderSets);
      break;
    }
    case ReAcsel::WrongACSELVersion: {
      QMessageBox::information(
        this, 
        tr("Information"), 
        tr("This ACSEL Bundle is not compatible with this version of Reality")
      );
      return;
    }
    case ReAcsel::SyntaxError: {
      QMessageBox::information(
        this, 
        tr("Information"), 
        tr("This ACSEL Bundle is corrupted")
      );
      return;
    }
    case ReAcsel::NotAnACSELBundle: {
      QMessageBox::information(
        this, 
        tr("Information"), 
        tr("The file is not an ACSEL Bundle")
      );
      return;
    }
    case ReAcsel::CannotAddShaderSet: {
      QMessageBox::information(
        this, 
        tr("Information"), 
        tr("It was not possible to save a shader set from the bundle")
      );
      return;
    }
    case ReAcsel::UnAuthorizedUpdate: {
      QMessageBox::information(
        this, 
        tr("Information"), 
        tr("A preset was attempted to be updated by a user other than the "
           "original author. That operation is not allowed.")
      );
      return;
    }
    default:
      break;
  }
  updateUI();
  stats->setText(tr("Bundle imported"));
  usSelector->displayShaders();
}

void ReAcselManager::exportShaders( const bool saveAcselShaders, 
                                    const bool saveUniversalShaders) 
{
  QString fileName = QFileDialog::getSaveFileName(
    this,
    tr("Enter a file name for the shader bundle"),
    "",
    tr("Reality Shader Bundle (*.rsb)")
  );
  if (fileName.isEmpty()) {
    return;
  }
  // Get the author's name from the Reality configuration 
  auto cfg = RealityBase::getConfiguration();
  QString author = cfg->value(RE_CFG_AUTHOR_NAME,"").toString();
  if (author.isEmpty()) {
    author = QInputDialog::getText(
      this, 
      tr("Please enter a name for the author"),
      tr("Author's name")
    );
  }
  if (author.isEmpty()) {
    return;
  }
  QString title = QInputDialog::getText(
    this, 
    tr("Please enter a title for this shader bundle"),
    tr("Title for the bundle")
  );
  if (title.isEmpty()) {
    return;
  }

  QStringList sets, universalShaders;

  if (saveAcselShaders) {
    auto items = shaderList->selectedItems();
    int count = items.count();
    for (int i = 0; i < count; i++) {
      sets.append( items[i]->data(0, Qt::UserRole).toString() );
    }
  }

  if (saveUniversalShaders) {
    universalShaders = usSelector->getSelectedShaderIDs();
  }

  QString result = ReAcsel::getInstance()->exportToBundle(
                     sets, universalShaders, title, author, fileName
                   );
  if (result != "") {
    QMessageBox::information(this, tr("Information"), result);  
  }

};


void ReAcselManager::editShaderSet() {
  if (btnACSELEdit->text() == tr("Save")) {
    auto item = shaderList->selectedItems()[0];
    QString setID = item->data(0, Qt::UserRole).toString();
    QVariantMap data;
    data["SetName"] = setName->text().trimmed();
    data["Description"] = setDescription->toPlainText().trimmed();
    data["IsDefault"] = setIsGeneric->isChecked();
    data["IsEnabled"] = setIsEnabled->isChecked();
    ReAcsel::getInstance()->updateSetProperties(setID, data);
    btnACSELEdit->setText(tr("Edit"));
    editControls->setEnabled(false);
    updateUI();
  }
  else {
    btnACSELEdit->setText(tr("Save"));    
    editControls->setEnabled(true);
  }
};

void ReAcselManager::switchTab( int which ) {
  // Currently the share tab is disabled and this code, therefore, doesn't
  // execute
  auto wdg = tabs->currentWidget();
  if ( wdg == ACSEL_Share_page) {
    if (!urlLoaded) {
      webView->load(QUrl("http://acsel.preta3d.com"));
      webView->show();
      urlLoaded = true;
    }
  }
}

void ReAcselManager::handleSelection( const QItemSelection& selected, 
                                      const QItemSelection& deselected )
{
  int numRows = shaderList->selectionModel()->selectedRows().count();
  btnACSELEdit->setEnabled(numRows == 1);
  if (numRows == 1) {
    auto item = shaderList->selectedItems()[0];
    setName->setText(item->text(0));
    setDescription->setPlainText(item->text(1));
    setIsGeneric->setChecked(item->text(3) == tr("Yes"));
    setIsEnabled->setChecked(item->text(4) == tr("Yes"));
  }
  else {
    setName->setText("");
    setDescription->setPlainText("");
    setIsGeneric->setChecked(false);
    setIsEnabled->setChecked(false);
  }
}

void ReAcselManager::closeEvent( QCloseEvent *event ) {
  ReConfigurationPtr settings = RealityBase::getConfiguration();
  settings->beginGroup(RE_ACSEL_MGR_CONFIG_KEY);
  settings->setValue("size", size());
  settings->setValue("pos", pos());
  settings->endGroup();
  settings->sync();
}


void ReAcselManager::copyShaderList() {
  auto items = shaderList->selectedItems();
  QString sets;
  int count = items.count();
  for (int i = 0; i < count; i++) {
    sets += QString("%1 - %2\n")
              .arg(items[i]->text(0))
              .arg(items[i]->data(0, Qt::UserRole).toString());
  }
  qApp->clipboard()->setText(sets);
}
