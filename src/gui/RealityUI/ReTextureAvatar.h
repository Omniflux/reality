/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_TEXTURE_AVATAR_H
#define RE_TEXTURE_AVATAR_H


#include <QWidget>
#include <QMouseEvent>
#include <QPixmap>
#include <QMenu>
#include <QSharedPointer>
#include <QApplication> 
#include <QClipboard> 

#include "ReTexture.h"
#include "ReTextureSelector.h"
#include "textures/ReMath.h"
#include "textures/ReConstant.h"
#include "textures/ReImageMap.h"
#include "textures/ReMix.h"
#include "textures/ReFresnelColor.h"
#include "textures/ReDistortedNoise.h"
#include "textures/ReColorMath.h"
#include "ui_textureAvatar.h"
#include "ReLogger.h"

#ifndef QT_CUSTOM_WIDGET
  #include "ReTextureSelector.h"
#endif

#define RE_CM_MULT "M"
#define RE_CM_ADD  "A"
#define RE_CM_SUB  "S"
#define RE_CM_NONE " "

using namespace Reality;

/**
 * Class: ReTextureAvatar
 *
 * The Texture Avatar is a simple widget that has the purpose of represent a
 * texture in the Reality UI. Since textures in Reality 3 are much more complex than
 * simply representing Image Maps, the UI implements a two-step system for editing
 * textures. The "Avatar" is a placeholder that represents the texture. When the
 * user clicks on the avatar then the Texture Editor is opened and an exitor appropriate
 * for the kind of texture is shown.
 * 
 * The Avatar itself uses a stacked widget to quickly present the kind of widgets
 * that best represent a certain type of texture
 */
class ReTextureAvatar: public QWidget, public Ui::wdg_textureAvatar {
  Q_OBJECT

  Q_PROPERTY(QString caption READ getCaption WRITE setCaption)
  Q_PROPERTY(QString ID READ getID WRITE setID)
  Q_PROPERTY(QString breadcrumb READ getBreadcrumb WRITE setBreadcrumb)

private:
  //! Each texture avatar is identified in a unique way.
  //! This ID is used by the texture avatar to communitate
  //! to the rest of the UI when the user requests to edit
  //! the texture linked by the avatar. The edit(QString&) 
  //! signal returns the ID of the avatar. Typically this is
  //! something like glossy.diffuse or glass.transmission
  QString ID;

  QString breadcrumb;

  //! The allowed data types. This is a mask compatible with
  //! ReTextureDataTypes
  quint8 allowedDataTypes;

  ReTexturePtr texture;
  ReTextureContainer* container;

  bool isBlank;
  bool deleteEnabled;

  // The menu attached to the gear button
  static QMenu* contextMenu;

  // The sub menu anchored to "New..."
  static QMenu* newTextureMenu;

  // static QAction* newAction;
  static QAction* editAction; 
  static QAction* deleteAction;
  static QAction* linkAction;
  static QAction* imageMapAction;
  static QAction* copyAction;
  static QAction* pasteAction;
  // Provides a textual representation of each texture and associates 
  // the correct type
  static QHash<QString, ReTextureType> textureTable;

  static QPixmap* textureColor;
  static QPixmap* textureGS;

  static ReTextureSelector* textureSelector;

  //! Create the menu attached to the gear button. The menu is where the 
  //! user can make the choice of creating, deleting or selecting a texture.
  void createMenu();

public:
  /**
   * ctor
   */
  explicit ReTextureAvatar( QWidget* parent = 0);

  virtual ~ReTextureAvatar();
  
  QSize sizeHint() const;

  //! Determines if the Delete option in the context menu is available or not.
  //! Default is to have it enabled.
  void setDeleteEnabled( const bool status );

  //! Returns whether this avatar allows to delete the connected texture.
  bool hasDeleteEnabled();

private slots:
  //! Processes the selection of the context menu attached
  //! to the gear button. All the menu operations like New, 
  //! Edit and Delete are handled by this method.
  void handleContextMenu(QAction* action);
  void gearMenuClicked();
  void filterMenuOptions();

public slots:
  
  /**
   * Method: setCaption
   */
  void setCaption( const QString newVal ) {
    caption->setText(newVal);
  };

  QString getCaption()  {
    return caption->text();
  };

  void setTextureTypeLabel(QString newVal) {
    lblTextureType->setText(newVal);
  }

  //! Method to make a texture appear blank. This is used when an avatar is present in 
  //! a widget but it doesn't contain data.
  void setBlank() {
    texture.clear();
    isBlank = true;
    update();
  }

protected:
  /**
   Traps the mouse release event so that we give the option to the user
   to edit a texture by simply clicking on it, and not just by clicking
   on the gear icon next to the widget.
   */
  void mouseReleaseEvent( QMouseEvent* event );

  void contextMenuEvent( QContextMenuEvent* /* event */);

private:

  void updateAvatar();

  void setLabelAvatar( ReTexturePtr tex, QLabel* lbl, QLabel* lbl2 = NULL );
  void setConstantAvatar( QLabel* lbl , ReConstantPtr tex );
  void setImageMapAvatar( QLabel* lbl, ImageMapPtr tex );
  void setColorMathAvatar( QLabel* lbl, QLabel* lbl2, ReColorMathPtr tex );
  void setMathAvatar( QLabel* lbl, QLabel* lbl2, ReMathPtr tex );
  void setFresnelAvatar( QLabel* lbl , ReFresnelColorPtr tex );
  void setMixAvatar( QLabel* lbl, QLabel* lbl2, ReMixTexturePtr tex );
  void setDistortedNoiseAvatar( QLabel* lbl , ReDistortedNoisePtr tex );

  /**
   * Given a color it initializes a string with the values for r,g and b, plus
   * it sets a string with the stylesheet to shet the background of a label
   * with the color passed in the first parameter.
   */
  void getColorValues( const QColor& color, QString& colorString, QString& styleSheet );

public:
  /*
   * Method: setTexture
   */
  void setTexture( const ReTexturePtr newTex, ReTextureContainer* tCont );

  inline void setAllowedDataTypes( const quint8 dTypes ) {
    allowedDataTypes = dTypes;
  }
  /*
   Method: getID
   */
  inline QString getID() {
    return ID;
  };
  /*
   Method: setID
   */
  inline void setID( QString newVal ) {
    ID = newVal;
  };

  /*
   Method: getBreadcrumb
   */
  inline QString getBreadcrumb() {
    return breadcrumb;
  };
  /*
   Method: setBreadcrumb
   */
  inline void setBreadcrumb( QString newVal ) {
    breadcrumb = newVal;
  };

  inline bool hasTexture() {
    return !texture.isNull();
  }

public slots:

  void paintEvent( QPaintEvent* event ) {
    updateAvatar();
    QWidget::paintEvent(event);
  }

signals:
  //! Signal: edit
  //!   Emitted when the widget signals the intention of the user to edit
  //!   this texture.
  void editTexture(ReTexturePtr tex, QString& breadcrumb);

  //! Emitted when a texture supports direct editing. Direct editing means
  //! that we show a simple editor, like a color picker, when the
  //! texture is simple enough that a single value is needed to change it.
  void directEdit( ReTexturePtr tex, QString& breadcrumb );

  //! Signal: addNewTexture
  //!   Emitted when the user selects to add a new texture.
  //! Parameters
  //!   ID - The identifier of the texture
  //!   texType - A string that indicates what kind of texture 
  //!             has been requested. It can be something like "ImageMap", "Constant"
  //!             etc.
  void addNewTexture(const QString& ID, const ReTextureType textureType);

  //! Signal: addNewImageMap
  //!   Emitted when the user selects to add a new image map.
  //! \param ID - The identifier of the texture
  void addNewImageMap(const QString& ID);

  //! Signal: linkTexture
  //!   Emitted when the user selects to link an existing texture to a material's channel.
  //! Parameters
  //!   ID - The identifier of the texture
  //!   textureName - A string that indicates the texture 
  void linkTexture(const QString& ID, const QString& textureName);

  //! Signal: linkTexture
  //!   Emitted when the user selects to detach an existing texture from a material's channel.
  //! Parameters
  //!   ID - The identifier of the texture
  void unlinkTexture(const QString& ID);

  //! Emitted when the user selected to copy a texture to the clipboard
  //! \param tex The texture to copy to the clipbaord
  void copyTexture(const ReTexturePtr& tex);

  //! Emitted when the user selected to paste a texture from the clipboard
  //! \param tex A pointer a texture that will point to a new texture created 
  //!            from the clipboard data.
  void pasteTexture(ReTexturePtr& tex);
};


#endif
