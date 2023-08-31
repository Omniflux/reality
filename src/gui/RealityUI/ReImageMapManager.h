/**
 * \file
 *  Reality plug-in (r)
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef REIMAGE_MAP_MANAGER_H
#define REIMAGE_MAP_MANAGER_H

#include <QAction>
#include <QMenu>
#include <QPixMap>
#include <QSharedPointer>
#include <QWidget>

#include "ReTexture.h"
#include "ui_imageMapManager.h"

namespace Reality {
  enum RGBChannel;
}


using namespace Reality;

class ReImageMapManager: public QWidget, public Ui::wdg_ReImageMapManager {
  Q_OBJECT

  Q_PROPERTY(QString label READ getLabel WRITE setLabel)
  Q_PROPERTY(QString textureFileName READ getTextureFileName WRITE setTextureFileName)

public:
  /**
   * ctor
   */
  explicit ReImageMapManager( QWidget* parent = 0);
  QSize sizeHint() const;

private:
  // Popup menu Actions
  QSharedPointer<QAction> newTextureAction, 
                          showInFolderAction;
protected:
  QMenu textureMenu;
  QString fileName;
  QPixmap texPreview;
  ReTexture::ReTextureDataType dataType;
  RGBChannel rgbChannel;
  bool isNormalMap;
  //! Used to save the gain used for the image map
  float gain;

public:
  void setImageClass( const ReTexture::ReTextureDataType dtype );

  inline float getGain() const {
    return gain;
  }

private slots:
  void showInFileManager(const QString &pathIn);
  /**
   * This method gives the user the option to open the OS' file manager
   * positioned on the texture file inside the folder that contains it.
   */
  void showTextureInFolder();

public slots:
 
  void setLabel(QString newLabel);
  QString getLabel();

  QString getTextureFileName();

  void setRgbChannel( const RGBChannel newVal );

  /**
   * This method gives the user the option to select the texture file
   */
  void selectNewTexture();

  void setNormalMap( bool yesNo );

  void setGain( const float g );

  //! Updates the text field to show the current file name. The name is abbreviated
  //! by replacing the path with "..."
  void showTextureFileName();

  //! Set the new name for the texture file.
  void setTextureFileName(QString newName);

  void updatePreview();

signals:
  void mapHasChanged();
};

#endif
