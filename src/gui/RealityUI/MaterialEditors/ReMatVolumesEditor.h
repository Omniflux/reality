/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REMAT_VOLUMES_EDITOR_H
#define REMAT_VOLUMES_EDITOR_H

#include <QStringListModel>

#include "ui_meMatVolumes.h"

namespace Reality {
  class ReMaterial;
}


/**
  Editor that allows the user to specify the volumes linked to a give material
 */
using namespace Reality;

class ReMatVolumesEditor : public QWidget,
                           public Ui::meMatVolumes {
 
  Q_OBJECT

private:
  ReMaterial* material;
  QStringListModel model;
  bool updatingUI;
                             
  void updateUI();

public:
  // Constructor: ReMatVolumesEditor
  explicit ReMatVolumesEditor( QWidget* parent = 0 );

  // Destructor: ReMatVolumesEditor
  ~ReMatVolumesEditor();

  QSize sizeHint() const;

  void setMaterial( ReMaterial* mat );

public slots:
  //! Responds to the signal that the material's volume selection has changed
  void materialVolumeChanged(const QString& volName, bool onOff);

private slots:
  void selectInnerVolume(const QModelIndex& current);
  void selectOuterVolume(const QModelIndex& current);
  void deselectOuter();
  //! Deselect the inner volume of the material
  void deselectInner();

signals:
  //! Emitted when a volume is selected
  void materialUpdated( QString objectID, 
                        QString materialName, 
                        QString propertyName, 
                        QVariant value );

};


#endif
