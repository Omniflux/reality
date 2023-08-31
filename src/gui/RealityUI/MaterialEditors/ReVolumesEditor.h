/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_VOLUMES_EDITOR_H
#define RE_VOLUMES_EDITOR_H

#include <QStandardItemModel>

#include "ReVolumes.h"
#include "ui_meVolumes.h"


using namespace Reality;

/**
 * This event class is used to signal the Volume editor when it needs to 
 * refresh its list. Because of the way events happen in the UI, it's not
 * always possible to simply call methods directly. For example, objects that
 * are deleted might end up being actually destroyed after the call to 
 * ReSceneData::deleteObject() returns. To work around this timing issue 
 * we post this event in the application queue and we post it at the end.
 * In this way the refresh of the UI will be done after Qt processes all the
 * other events and we are guaranteed to fetch the most up-to-date version
 * of the volumes catalog.
 */
class ReVolumeListUpdatedEvent : public QEvent {

public:
  static int reEventType;

  //! Constructor: ReVolumeListUpdatedEvent
  ReVolumeListUpdatedEvent() : QEvent( static_cast<QEvent::Type>(reEventType) ) 
  {
  };

  //! Destructor: ReVolumeListUpdatedEvent
 ~ReVolumeListUpdatedEvent() {
  };
};

/**
 Editor for the Lux Volumes
 */
class ReVolumesEditor: public QWidget, public Ui::meVolumes {
  
  Q_OBJECT

private:

  ReVolumePtr currentVolume;

  QStandardItemModel model;

  void addVolumeToList( ReVolumePtr vol );

  //! Use to notify when we are setting up fields programmatically
  //! Whe it's set to true we avoid updating the volume data 
  bool inSetup;

public:
  /**
   * ctor
   */
  explicit ReVolumesEditor( QWidget* parent = 0);

  QSize sizeHint() const;

  bool event( QEvent* e );
 
  void loadData();

private slots:

  void addVolume();
  void deleteVolume();
  void updateScattered( bool onOff );
  void editVolume( const QModelIndex& index, bool isNew = false );
  void nameUpdated();
  void iorUpdated( double newVal );
  void clarityUpdated( double newVal );
  void directionRUpdated( int newVal );
  void directionGUpdated( int newVal );
  void directionBUpdated( int newVal );
  void volColorUpdated();
  void volScatteringUpdated();
  void scatteringScaleUpdated( double newVal );
  void absorptionScaleUpdated( double newVal );


signals:

  void volumeAdded( const QString& volumeName );
  void volumeDeleted( const QString& volumeName );
  void volumeRenamed( const QString& oldName, const QString& newName );
  void volumeUpdated( const QString& volName );
};


#endif
