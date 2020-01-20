/**
 *
 */

#ifndef RE_TEXTUREEDITOR_H
#define RE_TEXTUREEDITOR_H


#include <QWidget>

#include "ui_reTextureEditor.h"

#include "ReTexture.h"
#include "ReTextureChannelDataModel.h"
 
using namespace Reality;
/*
 Class: ReTextureEditor
 */

class ReTextureEditor: public QWidget, public Ui::reTextureEditor {
  Q_OBJECT

private:
  // Variable: pageNames
  //   A simple dictionary that is used to fill the combobox that
  //   allows the user to change type of texture. The disctionary
  //   allows us to use simbolic names to refer to a specific page
  //   instead of relying on string comparison, which can be easily
  //   broken by updates and translations.
  QHash<Reality::ReTextureType, QString> pageNames;
  ReTextureChannelDataModelPtr model;
public:
  
  /**
   * ctor
   */
  explicit ReTextureEditor( QWidget* parent = 0);
  QSize sizeHint() const;

  void resetBreadcrumbs();

private slots:

  void emitEdit(ReTexturePtr tex, QString& breadcrumb);

public slots:
  /*
   Method: showHelpPage 

   Shows the page with the instructions on how to edit a texture
   */
  void showHelpPage() const ;

  /*
   Shows a given page, identified by the enum <PageType>  
   */
  void showPage( const Reality::ReTextureType pType ) const;

  /*
   Method: changeTextureType

   This method changes the type of the current texture and converts one type
   to the selected new type. The data is the shown in the page that corresponds 
   to the new type
   */
  void changeTextureType( const QString& newTextureType );

  /**
   * Assigns a texture data model to this editor to be edited by the users. 
   * Calling this method causes the editor to display the texture in the 
   * corresponding page/editor.
   *
   *  \param model The data model. This is what interfaces the underlying texture with the UI. Each
   *               modification of the texture data causes signals to be emitted to alert other
   *               listerners of the changes made.
   *  \param breadcrumb The breadcrumb that symbolizes the texture.
   *  \param isRoot Boolean that indicates if this texture is the root of the 
   *                breadcrumb trial. If false the breadcrumb will be appended 
   *                and separated from the previous "crumb" by a "/"
   */
  void setModel( ReTextureChannelDataModelPtr model, const QString& breadcrumb, const bool isRoot );

  void startEditor();

signals:
  //! Signal: editTexture
  //!   Signal emitted when a texture needs to be edited. This is a recursive event, like when
  //!   the user cliks on the gear menu of a Color/Constant texture in the ColorMath texture editor
  //!   The result is generally to call this class again with the subtexture loaded
  void editTexture( ReTexturePtr tex, QString& breadcrumb, bool isRoot );

  //! Signal: refreshMaterialView
  //!   Emitted when the material editor is requested to reload its data. This is
  //!   normally caused by a texture being converted to a different type
  void refreshMaterialView();

  //! Signal: textureTypeHasChanged
  //!   Emitted when a texture type has been changed and the host-app needs to be
  //!   synchronized. This is delegated to the <RealityPanel> class
  void textureTypeHasChanged(const QString& textureName, const ReTextureType newTextureType);

  void makeNewTexture( const QString& objectID, 
                       const QString& materialName, 
                       const QString& channelName,
                       const QString& masterTexture,
                       const QString& newTexture,
                       const ReTextureType texType,
                       const ReTexture::ReTextureDataType dataType );
  //! Signal: linkTexture
  //! Emitted when the user links an existing texture to a texture's channel.
  //! Forwarded from ReAbstractTextrueEditor's subclasses
  void linkTexture( const QString& objectID,
                    const QString& materialID, 
                    const QString& channelID, 
                    const QString& textureName,
                    const QString& subTextureName );  

  //! Emitted when a sub-texture is removed from the main texture
  void unlinkTexture( const QString& objectID, 
                      const QString& materialID, 
                      const QString& textureName,
                      const QString& channelName );

  void previousBreadcrumb( ReTexturePtr tex );

  //! Emitted when the modifiers tab needs to be refreshed in response
  //! to the user making changes in the texture in the editor. This signal
  //! is a forwarded signal from the underlying texture editor
  void updateModifiers();

  //! Emitted when a texture is pasted, as in copy/paste.
  void replaceTexture( const QString& channelID,                      
                       const QString& jsonTextureData,
                       const QString& masterTextureName );
};

#endif
