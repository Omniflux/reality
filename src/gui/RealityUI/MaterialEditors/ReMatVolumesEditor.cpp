/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReGeometryObject.h"
#include "ReMaterial.h"
#include "ReMatVolumesEditor.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"

ReMatVolumesEditor::ReMatVolumesEditor(  QWidget* parent  ) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  innerVolumes->setModel(&model);
  outerVolumes->setModel(&model);
  // Get notified whenever a volume is selected
  connect(innerVolumes,SIGNAL(clicked(const QModelIndex&)),
          this, SLOT(selectInnerVolume(const QModelIndex&)));
  connect(outerVolumes,SIGNAL(clicked(const QModelIndex&)),
          this, SLOT(selectOuterVolume(const QModelIndex&)));
  
  connect(btnDeselectInner,SIGNAL(clicked()),this, SLOT(deselectInner()));
  connect(btnDeselectOuter,SIGNAL(clicked()),this, SLOT(deselectOuter()));
  updatingUI = false;
}

ReMatVolumesEditor::~ReMatVolumesEditor() {

}

QSize ReMatVolumesEditor::sizeHint() const {
  return QSize(525,225);
}

void ReMatVolumesEditor::updateUI() {
  auto volumeList = model.stringList();
  QString innerVolName = material->getInnerVolume();
  QString outerVolName = material->getOuterVolume();
  int innerIndex = 0, outerIndex = 0;
  innerIndex = volumeList.indexOf(innerVolName);
  outerIndex = volumeList.indexOf(outerVolName);

  updatingUI = true;
  innerVolumes->clearSelection();
  outerVolumes->clearSelection();
  if (innerIndex != -1) {
    innerVolumes->selectionModel()->select(model.index(innerIndex), QItemSelectionModel::Select);
  }
  if (outerIndex != -1) {
    outerVolumes->selectionModel()->select(model.index(outerIndex), QItemSelectionModel::Select);
  }
  updatingUI = false;

}


void ReMatVolumesEditor::setMaterial( ReMaterial* mat ) {
  material = mat;

  model.removeRows(0, model.rowCount());

  ReVolumeIterator i(RealitySceneData->getVolumes());
  QStringList volumeList;
  while( i.hasNext() ) {
    i.next();
    QString volName = i.key();
    volumeList << volName;
  }
  model.setStringList(volumeList);
  updateUI();
}

void ReMatVolumesEditor::selectInnerVolume(const QModelIndex& current) 
{
  if (!updatingUI) {
    QString val = model.data(current, Qt::DisplayRole).toString();
    material->setInnerVolume(val);
    emit materialUpdated( material->getParent()->getInternalName(), 
                          material->getName(),
                          "innerVolume",
                          val );
  }
}

void ReMatVolumesEditor::materialVolumeChanged(const QString& volName, bool onOff) {
  updateUI();
}

void ReMatVolumesEditor::selectOuterVolume(const QModelIndex& current) 
{
  if (!updatingUI) {
    QString val = model.data(current, Qt::DisplayRole).toString();
    material->setOuterVolume(val);
    emit materialUpdated( material->getParent()->getInternalName(), 
                          material->getName(),
                          "outerVolume",
                          val );
  }
}

void ReMatVolumesEditor::deselectInner() {
  if (!updatingUI) {
    innerVolumes->selectionModel()->clearSelection();
    material->setInnerVolume("");
    emit materialUpdated( material->getParent()->getInternalName(), 
                          material->getName(),
                          "innerVolume",
                          "" );
  }
};

void ReMatVolumesEditor::deselectOuter() {
  if (!updatingUI) {
    outerVolumes->selectionModel()->clearSelection();
    material->setOuterVolume("");
    emit materialUpdated( material->getParent()->getInternalName(), 
                          material->getName(),
                          "outerVolume",
                          "" );
  }
};


