\
/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReCameraEditor.h"

#include <QJson/Parser>

#include "RealityBase.h"
#include "ReCamera.h"
#include "ReLogger.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"


#define RL_COL_NAME     0
#define RL_COL_FOCAL    1
#define RL_COL_CURRENT  2

ReCameraEditor::ReCameraEditor( QWidget* parent) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

  addFRFs();

  // Get notified whenever a camera is selected
  connect(cameraList, SIGNAL(itemSelectionChanged()), this, SLOT(updateCameraSelected()));

  // Check/uncheck the manual exposure box
  connect(gbExposure, SIGNAL(toggled(bool)), this, SLOT(updateExposureEnabled(bool)));
  // Check/uncheck the DOF box
  connect(gbDOF, SIGNAL(toggled(bool)), this, SLOT(updateDOFEnabled(bool)));

  // Select a Film speed
  connect(filmISO, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, 
          SLOT(updateFilmISO(QListWidgetItem*)));

  // Select a shutter speed
  connect(shutter, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, 
          SLOT(updateShutter(QListWidgetItem*)));
  // Select an f-stop
  connect(fstop, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, 
          SLOT(updateFStop(QListWidgetItem*)));
  // Select a DOF f-stop
  connect(dofStop, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, 
          SLOT(updateDOFStop(QListWidgetItem*)));

  // Select a Film Response Function
  connect(frfList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, 
          SLOT(updateFRF(QListWidgetItem*)));
  userSelectedCamera = false;
  currentCamera.clear();
  shutterPresets.initTable();
  shutterPresets.initWidget(shutter);

  updatingUI = false;
}

void ReCameraEditor::addFRFs() {
  QFile frfs(":/textResources/frf.json");
  if (!frfs.open(QIODevice::ReadOnly)) {
    RE_LOG_DEBUG() << "Error: cannot read the list of FRFs.";
    return;
  }
  QString frfText = frfs.readAll();
  frfs.close();

  QJson::Parser parser;
  bool ok;

  QVariantMap data = parser.parse(frfText.toUtf8(), &ok).toMap();
  if (!ok) {
    RE_LOG_DEBUG() << "Error: FRF data is not formatted correctly.";
    return;
  }
  QMapIterator<QString, QVariant> i(data);
  int row = 0;
  while( i.hasNext() ) {
    i.next();
    QListWidgetItem* item = new QListWidgetItem(i.value().toString());
    item->setData(Qt::UserRole, i.key());
    frfList->insertItem(row, item);
    row++;
  }
  QListWidgetItem* item = new QListWidgetItem(tr("None"));
  item->setData(Qt::UserRole, "NONE");
  frfList->insertItem(0, item);
}

void ReCameraEditor::loadCameraData() {
  updatingUI = true;
  cameraList->clear();
  QTreeWidgetItem* header = cameraList->headerItem();
  header->setText(2, QString(tr("%1 current")
                     .arg(RealityBase::getRealityBase()->getPrintableHostAppID())));

  ReCameraIterator i(*RealitySceneData->getCameras());

  while( i.hasNext() ){
    i.next();
    QTreeWidgetItem* item = new QTreeWidgetItem(cameraList);
    item->setText(0,i.value()->getName());
    item->setToolTip(0,QString("Internal ID: %1").arg(i.value()->getID()));
    // Associate the camera unique ID with the item for later recall
    item->setData(0, Qt::UserRole, i.key());
    item->setText(1,QString("%1").arg(i.value()->getFocalLength(),0,'f',2));
    cameraList->addTopLevelItem(item);
  }
  frfList->setEnabled(RealitySceneData->getNumCameras() > 0);
  cameraChanged(RealitySceneData->getSelectedCameraID());
  updatingUI = false;
}


//! Called when a camera data changed in the host app scene while
//! Reality is running
void ReCameraEditor::cameraChanged(const QString cameraID) {
  // RE_LOG_INFO() << "Camera data changed: " << cameraID;
  ReCameraPtr cam = RealitySceneData->getCamera(cameraID);
  if (cam.isNull()) {
    return;
  }
  updatingUI = true;
  // Remove the flag from the previously-marked current item
  QList<QTreeWidgetItem *> items = cameraList->findItems("*",Qt::MatchExactly,2);
  if (items.count() > 0) {
    items[0]->setText(2,"");
  }

  items = cameraList->findItems(cam->getName(),Qt::MatchExactly);
  items[0]->setText(1,QString::number(cam->getFocalLength(),'f',2));
  // Mark this item as current
  items[0]->setText(2,"*");
  // If the user has not selected a camera we select the current one automatically
  if (!userSelectedCamera) {
    cameraList->clearSelection();
    QAbstractItemModel* model = cameraList->model();
    QModelIndexList matches = model->match(model->index(0,0), 
                                           Qt::DisplayRole, 
                                           cam->getName());
    // This should not happen but just in case...
    if (matches.count() == 0) {
      return;
    }
    cameraList->setCurrentItem(items[0]);
    cameraList->selectionModel()->select(matches[0], 
                                         QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    QString cameraID = items[0]->data(0, Qt::UserRole).toString();
    // RealitySceneData->selectCamera(cameraID);
    currentCamera = RealitySceneData->getCamera(cameraID);
    // Notify everybody which camera is now active
    emit cameraSelected(cameraID);
  }
  
  showFocalDistance();
  updatingUI = false;
}

void ReCameraEditor::syncCameraSelectionWithHost(const QString cameraID) {
  ReCameraPtr cam = RealitySceneData->getCamera(cameraID);
  if (cam.isNull()) {
    return;
  }
  updatingUI = true;
  QList<QTreeWidgetItem *> items = cameraList->findItems("*",Qt::MatchExactly,2);
  QTreeWidgetItem* item = NULL;
  if (items.count() > 0) {
    item = items[0];
  }
  if (item == NULL) {
    return;
  }

  item->setText(2,"");

  items = cameraList->findItems(cam->getName(),Qt::MatchExactly);
  item = items[0];
  item->setText(1,QString::number(cam->getFocalLength(),'f',2));
  // Mark this item as current
  item->setText(2,"*");
  // RE_LOG_INFO() << "Synching the UI to use camera " << cameraID << " from host";
  cameraList->clearSelection();  
  item->setSelected(true);
  currentCamera = RealitySceneData->getCamera(cameraID);  
  updatingUI = false;
}


void ReCameraEditor::showCameraProperties() {
  // Film ISO
  QList<QListWidgetItem *> items = filmISO->findItems(
    QString::number(currentCamera->getISO()),
    Qt::MatchStartsWith
  );
  if (items.count() > 0) {
    filmISO->setCurrentItem(items[0]);
  }
  // Shutter speed
  float shutterSpeed = currentCamera->getShutter();
  QString shutterValStr = shutterPresets.valueToString(shutterSpeed);

  items = shutter->findItems(shutterValStr, Qt::MatchExactly);

  if (items.count() > 0) {
    shutter->setCurrentItem(items[0]);
  }
  else {
    shutter->setCurrentRow(0, QItemSelectionModel::Select);
  }
  // f-stop
  items = fstop->findItems(
     QString("%1").arg(currentCamera->getFStop(),0,'f',1),
    Qt::MatchExactly
  );
  if (items.count() > 0) {
    fstop->setCurrentItem(items[0]);
  }
  // dof f-stop
  items = dofStop->findItems(
     QString("%1").arg(currentCamera->getDOFStop(),0,'f',1),
    Qt::MatchExactly
  );
  if (items.count() > 0) {
    dofStop->setCurrentItem(items[0]);
  }

  // Film Response
  QAbstractItemModel*  model =  frfList->model(); 
  QModelIndexList frfItems = model->match(model->index(0,0), 
                                          Qt::UserRole, 
                                          currentCamera->getFilmResponse());
  frfList->setCurrentItem(frfList->item(frfItems[0].row()));
  // DOF
  gbDOF->setChecked(currentCamera->isDOFEnabled());
}

void ReCameraEditor::updateCameraSelected() {
  auto items = cameraList->selectedItems();
  QTreeWidgetItem* item = NULL;
  
  if (items.count() > 0) {
    item = items[0];
  }
  if (!item) {
    return;
  }
  if (!updatingUI) {
    userSelectedCamera = true;    
  }
  updatingUI = true;
  QString cameraID = item->data(0, Qt::UserRole).toString();
  currentCamera = RealitySceneData->getCamera(cameraID);
  // Show the data
  gbExposure->setChecked(currentCamera->isExposureEnabled());
  showFocalDistance();
  showCameraProperties();
  emit cameraSelected(cameraID);
  updatingUI = false;
}

void ReCameraEditor::updateFilmISO(QListWidgetItem* item) {
  if (updatingUI) {
    return;
  }
  int newISO = item->text().toInt();
  currentCamera->setISO(newISO);
  emit cameraChanged(currentCamera->getID(), "iso", newISO);  
}

void ReCameraEditor::updateShutter(QListWidgetItem* item) {
  if (updatingUI) {
    return;
  }
  QString tShutter = item->text();
  float shutterSpeed;
  if (tShutter.startsWith("1/")) {
    tShutter.replace("1/", "");
    // Truncated to the third deciman point because that's how LuxRender
    // like it.
    shutterSpeed = floor(1 / tShutter.toFloat()*1000 )/1000;
  }
  else {
    // The shutter speed is > 0.x
    shutterSpeed = tShutter.toFloat();
  }
  currentCamera->setShutter(shutterSpeed);
  emit cameraChanged(currentCamera->getID(), "shutter", shutterSpeed);  
}

void ReCameraEditor::updateFStop(QListWidgetItem* item) {
  if (updatingUI) {
    return;
  }
  float newfStop = item->text().toFloat();
  currentCamera->setFStop(newfStop);
  emit cameraChanged(currentCamera->getID(), "fStop", newfStop);  
}

void ReCameraEditor::updateDOFStop(QListWidgetItem* item) {
  if (updatingUI) {
    return;
  }
  if (currentCamera.isNull()) {
    return;
  }
  float newDofStop = item->text().toFloat();
  currentCamera->setDOFStop(newDofStop);
  emit cameraChanged(currentCamera->getID(), "dofStop", newDofStop);  
}

void ReCameraEditor::updateFRF(QListWidgetItem* item) {
  if (updatingUI) {
    return;
  }
  QString newFRF = item->data(Qt::UserRole).toString();
  currentCamera->setFilmResponse(newFRF);
  emit cameraChanged(currentCamera->getID(), "filmResponse", newFRF);  
}

void ReCameraEditor::updateExposureEnabled( bool onOff ) {
  if (currentCamera.isNull()) {
    gbExposure->setChecked(false);
    return;
  }

  currentCamera->setExposureEnabled(onOff);
  emit cameraChanged(currentCamera->getID(), "exposureEnabled", onOff);  
}

void ReCameraEditor::updateDOFEnabled( bool onOff ) {
  if (currentCamera.isNull()) {
    gbDOF->setChecked(false);
    return;
  }
  currentCamera->setDOFEnabled(onOff);
  emit cameraChanged(currentCamera->getID(), "dofEnabled", onOff);  
}

void ReCameraEditor::showFocalDistance() {
  focalDistance->setText(
    QString::number( currentCamera->getFocalDistance() )
  );
}
