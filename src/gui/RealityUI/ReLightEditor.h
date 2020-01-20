/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RELIGHT_EDITOR_H
#define RELIGHT_EDITOR_H

#include <QtCore>
#include <QSharedPointer>
#include <QIcon>
#include "ReLight.h"
#include "ReTextureEditor.h"

#include "ui_reLightEditor.h"

using namespace Reality;

class ReLightEditor: public QWidget, 
                     public Ui::lightEditor {
  Q_OBJECT

private:
  bool updatingUI;
  ReLightPtr currentLight;
  // QSharedPointer<QIcon> sunIcon;

  //! Adds one light to the list
  void addLightToList( ReLightPtr light );

public:
  explicit ReLightEditor( QWidget* parent = 0);


  void updateUI();

  QSize sizeHint() const {
    return QSize(350, 247);    
  }

  inline ReLightPtr getCurrentLight() {
    return currentLight;
  }

  void eraseAllLights();
  
private slots:
  void lightSelected();
  void updateColorModelEditor();
  void updateGain( double newVal );
  void updatePower( double newVal );
  void updateEfficiency( double newVal );
  void updateLightGroup(const QString& newVal );
  void updateColorTemperature(int newVal);
  void updateColorRGB();
  void updateConeAngle(double newVal);
  void updateConeFeather(double newVal);
  void updatePreset();
  void updateLightOnOff(QTreeWidgetItem* item, int column);
  void updateIBLMapFormat();
  void updateIBLMap();
  void updateGamma( double newVal );
  void updateIESMap( const QString& newMapName );
  void selectIESMap();
  void updateYRotation(double angle);
  void updateIBLFileName(const QString&);
  void updateSun(double value);
  void updateSkyStyle( bool onOff );
  void updateInvertDirection(bool isOn);
  void updateAlphaChannel(bool isOn);
  void updateImageFile(const QString& fName);
  void selectImageFile();

  //! Convert a light back to the original material
  void convertToMat();

public slots:
  void loadLightData();
  void lightDeleted( const QString& lightID );

  //! Called when the light has been updated in the database and
  //! the UI needs to reflect the change. 
  void updateLight( const QString& lightID );

signals:
  void lightChanged( const QString lightID, 
                     const QString valueName,
                     const QVariant value  );

  void convertToMaterial(const QString objectID, const QString matName, const QString lightID);

  // void editTexture( ReTexturePtr tex, QString& breadcrumb );

  // //! Emitted when the user adds a new texture to a material channel
  // void makeNewTexture( const QString& objectID, const QString& texturerName, const ReTextureType textureType );

  // //! Emitted when the user links an existing texture to a material's channel
  // void linkTexture( const QString& objectID,
  //                   const QString& materialID, 
  //                   const QString& channelID, 
  //                   const QString& textureName );

  // //! Emitted when the user detaches a texture from a material's channel
  // void unlinkTexture( const QString& objectID,
  //                     const QString& materialID, 
  //                     const QString& channelID );
};

#endif
