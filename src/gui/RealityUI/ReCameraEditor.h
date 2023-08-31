/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __RECAMERA_EDITOR_H__
#define __RECAMERA_EDITOR_H__

#include <QWidget>

#include "RePresetList.h"
#include "ui_reCameraEditor.h"

namespace Reality {
  class ReCamera;
  typedef QSharedPointer<ReCamera> ReCameraPtr;
}


using namespace Reality;

class ReShutterPresets : public RePresetList {
public:
  enum ShutterSpeeds {
    S_30_SEC,
    S_20_SEC,
    S_10_SEC,
    S_5_SEC,
    S_3_SEC,
    S_1_SEC,
    S_1_2_SEC,
    S_1_4_SEC,
    S_1_8_SEC,
    S_1_15_SEC,
    S_1_30_SEC,
    S_1_60_SEC,
    S_1_125_SEC,
    S_1_250_SEC,
    S_1_500_SEC,
    S_1_1000_SEC,
  };

  typedef QHash<int, float> ShutterValueTable;
  ShutterValueTable shutterValues;

  inline QString valueToString( const float speed ) {
    return presetMap[shutterValues.key(speed)];
  }

  void initTable() {
    presetMap[S_30_SEC]     = "30";
    presetMap[S_20_SEC]     = "20";
    presetMap[S_10_SEC]     = "10";
    presetMap[S_5_SEC]      = "5";
    presetMap[S_3_SEC]      = "3";
    presetMap[S_1_SEC]      = "1";
    presetMap[S_1_2_SEC]    = "1/2";
    presetMap[S_1_4_SEC]    = "1/4";
    presetMap[S_1_8_SEC]    = "1/8";
    presetMap[S_1_15_SEC]   = "1/15";
    presetMap[S_1_30_SEC]   = "1/30";
    presetMap[S_1_60_SEC]   = "1/60";
    presetMap[S_1_125_SEC]  = "1/125";
    presetMap[S_1_250_SEC]  = "1/250";
    presetMap[S_1_500_SEC]  = "1/500";
    presetMap[S_1_1000_SEC] = "1/1000";

    shutterValues[S_30_SEC]     = 30.0;
    shutterValues[S_20_SEC]     = 20.0;
    shutterValues[S_10_SEC]     = 10.0;
    shutterValues[S_5_SEC]      = 5.0;
    shutterValues[S_3_SEC]      = 3.0;
    shutterValues[S_1_SEC]      = 1.0;
    shutterValues[S_1_2_SEC]    = 0.5;
    shutterValues[S_1_4_SEC]    = 0.25;
    shutterValues[S_1_8_SEC]    = 0.125;
    shutterValues[S_1_15_SEC]   = 0.066;
    shutterValues[S_1_30_SEC]   = 0.033;
    shutterValues[S_1_60_SEC]   = 0.016;
    shutterValues[S_1_125_SEC]  = 0.008;
    shutterValues[S_1_250_SEC]  = 0.004;
    shutterValues[S_1_500_SEC]  = 0.002;
    shutterValues[S_1_1000_SEC] = 0.001;
  }

};

class ReCameraEditor: public QWidget, 
                      public Ui::cameraEditor {
  Q_OBJECT

private:
  bool updatingUI;
  ReCameraPtr currentCamera;

  //! Used to keep track when the user selectes a camera
  //! explicitly instead of going with the one selected in the
  //! host app.
  bool userSelectedCamera;

  void showCameraProperties();

  void showFocalDistance();
  //! Load the table of Film Response Functions.
  void addFRFs();

protected:
  ReShutterPresets shutterPresets;

public:
  explicit ReCameraEditor( QWidget* parent = 0);

  void updateUI();

  QSize sizeHint() const {
    return QSize(810, 550);
  }

private slots:
  void updateCameraSelected();
  void updateFilmISO(QListWidgetItem* item);
  void updateShutter(QListWidgetItem* item);
  void updateFStop(QListWidgetItem* item);
  void updateDOFStop(QListWidgetItem* item);
  void updateExposureEnabled( bool onOff );
  void updateDOFEnabled( bool onOff );
  void updateFRF(QListWidgetItem* item);
  
public slots:
  void loadCameraData();
  void cameraChanged(const QString cameraID);
  void syncCameraSelectionWithHost(const QString cameraID);

signals:
  void cameraChanged( const QString cameraID, 
                      const QString valueName,
                      const QVariant value  );

  // Emitted when the user explicitly selects a camera from the 
  // list instead of using the one selected in the host app
  void cameraSelected( const QString cameraID );
};

#endif
