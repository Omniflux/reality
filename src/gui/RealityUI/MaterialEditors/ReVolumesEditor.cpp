/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include <QWidget>
#include <QMessageBox>

#include "ReVolumesEditor.h"
#include "ReSceneDataGlobal.h"

// Assign a new event ID to our custom event
int ReVolumeListUpdatedEvent::reEventType = QEvent::registerEventType();

// Column positions for each data item in the list
#define COL_NAME      0
#define COL_TYPE      1
#define COL_IOR       2
#define COL_CLARITY   3  
#define COL_PROTECTED 4  

ReVolumesEditor::ReVolumesEditor( QWidget* parent ) : QWidget(parent) {
  setupUi(this);
  inSetup = false;

  editWidgets->setEnabled(false);
  volumeList->setModel(&model);
  btnDelete->setEnabled(false);  

  connect(btnAddVolume, SIGNAL(clicked()), this, SLOT(addVolume()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteVolume()));
  connect(scatteredFlag, SIGNAL(toggled(bool)), this, SLOT(updateScattered(bool)));

  // Get notified whenever a material is selected
  connect(volumeList,SIGNAL(clicked(const QModelIndex&)),this, SLOT(editVolume(const QModelIndex&)));

  // Connect to the selection model. This is necessary if we want to respond
  // to keyboard navigation events for the TreeView. The connection must be 
  // done after the model is set.
  connect(volumeList->selectionModel(), 
          SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
          this, 
          SLOT(editVolume(const QModelIndex&)));

  // Edit events
  connect(volumeName, SIGNAL(editingFinished()), this, SLOT(nameUpdated()));
  connect(ior,        SIGNAL(valueChanged(double)), this, SLOT(iorUpdated(double)));
  connect(clarity,    SIGNAL(valueChanged(double)), this, SLOT(clarityUpdated(double)));
  connect(directionR, SIGNAL(valueChanged(int)), this, SLOT(directionRUpdated(int)));
  connect(directionG, SIGNAL(valueChanged(int)), this, SLOT(directionGUpdated(int)));
  connect(directionB, SIGNAL(valueChanged(int)), this, SLOT(directionBUpdated(int)));
  connect(absorption, SIGNAL(colorChanged()),    this, SLOT(volColorUpdated()));
  connect(absorptionScale, SIGNAL(valueChanged(double)), this, SLOT(absorptionScaleUpdated(double)));
  connect(scattering, SIGNAL(colorChanged()),    this, SLOT(volScatteringUpdated()));
  connect(scatteringScale, SIGNAL(valueChanged(double)), this, SLOT(scatteringScaleUpdated(double)));

}

 

bool ReVolumesEditor::event( QEvent* e ) {
  if (e->type() == ReVolumeListUpdatedEvent::reEventType) {
    loadData();
    return true;
  }
  return false;
}

QSize ReVolumesEditor::sizeHint() const {
  return QSize(660,370);
}

void ReVolumesEditor::addVolumeToList( ReVolumePtr vol ) {
  QList<QStandardItem*> fields;

  QStandardItem* item = new QStandardItem(vol->getName());
  fields.append(item);
  fields.append(new QStandardItem(vol->getTypeAsString()));
  fields.append(new QStandardItem(QString::number(vol->getIOR())));
  fields.append(new QStandardItem(QString::number(vol->getClarityAtDepth())));
  fields.append(new QStandardItem((vol->isEditable() ? "" : "Yes")));
  model.appendRow(fields);
}

void ReVolumesEditor::addVolume() {
  editWidgets->setEnabled(true);
  QString newVolName = "<new volume>";
  currentVolume = ReVolumePtr( new ReVolume(newVolName) );
  RealitySceneData->saveVolume( currentVolume );

  addVolumeToList(currentVolume);
  int numRows = model.rowCount();
  QItemSelectionModel* selection = volumeList->selectionModel();

  selection->clear();
  QModelIndex startIndex = model.index(numRows-1, COL_NAME);
  QModelIndex endIndex = model.index(numRows-1, model.columnCount()-1);

  selection->select( QItemSelection (startIndex, endIndex), QItemSelectionModel::SelectCurrent );
  volumeList->setCurrentIndex(startIndex);

  emit volumeAdded(newVolName);
  editVolume(startIndex, true);
}

void ReVolumesEditor::deleteVolume() {
  if (QMessageBox::question(this, 
                            "Delete confirmation", 
                            QString("Do you want to delete volume %1?").arg(currentVolume->getName()), 
                            QMessageBox::Yes | QMessageBox::No, 
                            QMessageBox::No) == QMessageBox::Yes) {
    QString volName = currentVolume->getName();
    RealitySceneData->removeVolume(volName);
    currentVolume.clear();
    loadData();

    int numRows = model.rowCount();
    QItemSelectionModel* selection = volumeList->selectionModel();

    selection->clear();
    QModelIndex startIndex = model.index(numRows-1, COL_NAME);
    QModelIndex endIndex = model.index(numRows-1, model.columnCount()-1);

    selection->select(  QItemSelection (startIndex, endIndex),   QItemSelectionModel::SelectCurrent );
    volumeList->setCurrentIndex(startIndex);
    emit volumeDeleted(volName);
  }
}


/*
 Method: loadData

 Initializes the tree view with the list of volumes available.
 This method is meant to be called externally by the routine that
 calls <setMaterial>.
 */
void ReVolumesEditor::loadData() {
  model.clear();
  QStringList headers;
  headers << tr("Name") << tr("Type") << tr("IOR") << tr("Clarity") << tr("Protected");
  model.setHorizontalHeaderLabels(headers);

  ReVolumeIterator i(RealitySceneData->getVolumes());
  while( i.hasNext() ) {
    i.next();
    // Create the new top level group
    QStringList fields;
    ReVolumePtr vol = i.value();
    addVolumeToList(vol);
  }
}

void ReVolumesEditor::absorptionScaleUpdated( double newVal ) {
  if (inSetup) {
    return;
  }
  currentVolume->setAbsorptionScale(newVal);
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::nameUpdated() {
  if (inSetup) {
    return;
  }
  QModelIndex index = volumeList->selectionModel()->currentIndex();
  QStandardItem* item = model.item(index.row(), COL_NAME);
  QString oldName = currentVolume->getName();
  RealitySceneData->renameVolume(oldName, volumeName->text());

  if (item) {
    item->setText(volumeName->text());
  }
  emit volumeRenamed(oldName, volumeName->text());
}


void ReVolumesEditor::iorUpdated( double newVal ) {
  if (inSetup) {
    return;
  }
  currentVolume->setIOR(newVal);
  QModelIndex index = volumeList->selectionModel()->currentIndex();
  QStandardItem* item = model.item(index.row(),COL_IOR);
  if (item) {
    item->setText(QString::number(newVal));
  }
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::clarityUpdated( double newVal ) {
  if (inSetup) {
    return;
  }
  currentVolume->setClarityAtDepth(newVal);
  QModelIndex index = volumeList->selectionModel()->currentIndex();
  QStandardItem* item = model.item(index.row(),COL_CLARITY);
  if (item) {
    item->setText(QString::number(newVal));    
  }
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::directionRUpdated( int newVal ) {
  if (inSetup) {
    return;
  }
  currentVolume->setDirectionR(newVal/10.0);
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::directionGUpdated( int newVal ) {
  if (inSetup) {
    return;
  }
  currentVolume->setDirectionG(newVal/10.0);
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::directionBUpdated( int newVal ) {
  if (inSetup) {
    return;
  }
  currentVolume->setDirectionB(newVal/10.0);
  emit volumeUpdated(currentVolume->getName());
}


void ReVolumesEditor::editVolume( const QModelIndex& index, bool isNew  ) {
  // The item returned here could refer to any column, we need column 0, where the name is
  QStandardItem *item = model.item(index.row(), COL_NAME);

  inSetup = true;
  volumeName->setText("");
  ior->setValue(1.0);
  clarity->setValue(1.0);
  absorption->setColor(QColor(255,255,255));
  scatteredFlag->setChecked(false);
  scattering->setColor(QColor(0,0,0));
  directionR->setValue(0);
  directionG->setValue(0);
  directionB->setValue(0);

  if (!item) {
    return;
  }
  QString volumeNameStr = item->text();
  currentVolume = RealitySceneData->getVolume(volumeNameStr);
  if (!currentVolume.isNull()) {
    volumeName->setText(volumeNameStr);
    ior->setValue(currentVolume->getIOR());
    clarity->setValue(currentVolume->getClarityAtDepth());
    absorption->setColor(currentVolume->getColor());
    absorptionScale->setValue(currentVolume->getAbsorptionScale());
    
    if (currentVolume->isEditable()) {
      editWidgets->setEnabled(true);
      btnDelete->setEnabled(true);
    }
    else {
      editWidgets->setEnabled(false);
      btnDelete->setEnabled(false);
    }

    scatteredFlag->setChecked(currentVolume->getType() == ReVolume::homogeneous);

    scattering->setColor(currentVolume->getScatteringColor());
    scatteringScale->setValue(currentVolume->getScatteringScale());
    directionR->setValue((int)(currentVolume->getDirectionR()*10));
    directionG->setValue((int)(currentVolume->getDirectionG()*10));
    directionB->setValue((int)(currentVolume->getDirectionB()*10));
  }
  if (isNew) {
    volumeName->selectAll();    
  }
  inSetup = false;
}

void ReVolumesEditor::updateScattered( bool onOff ) {
  if (inSetup) {
    return;
  }
  QModelIndex index = volumeList->selectionModel()->currentIndex();
  QStandardItem* item = model.item(index.row(), COL_TYPE);
  if (item) {
    if (onOff) {
      currentVolume->setType(ReVolume::homogeneous);
      item->setText("scattered");
    }
    else {
      currentVolume->setType(ReVolume::clear);    
      item->setText("clear");
    }
  }
  else {
    return;
  }
  scattering->setEnabled(onOff);
  directionR->setEnabled(onOff);
  directionG->setEnabled(onOff);
  directionB->setEnabled(onOff);

  lblScattering->setEnabled(onOff);
  directionR->setEnabled(onOff);
  directionG->setEnabled(onOff);
  directionB->setEnabled(onOff);
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::volColorUpdated() {
  if (inSetup) {
    return;
  }
  currentVolume->setColor(absorption->getColor());
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::volScatteringUpdated() {
  if (inSetup) {
    return;
  }
  currentVolume->setScatteringColor(scattering->getColor());
  emit volumeUpdated(currentVolume->getName());
}

void ReVolumesEditor::scatteringScaleUpdated( double newVal ) {
  if (inSetup) {
    return;
  }
  currentVolume->setScatteringScale(newVal);
  emit volumeUpdated(currentVolume->getName());
}


