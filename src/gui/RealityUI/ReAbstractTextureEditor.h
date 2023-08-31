/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REABSTRACT_TEXTURE_EDITOR_H
#define REABSTRACT_TEXTURE_EDITOR_H

#include <QWidget>

#include "ReTexture.h"

namespace Reality {
  enum ReTextureType;
  class ReMaterial;
  class ReTextureChannelDataModel;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
}


using namespace Reality;

/**
 * Base class for all the texture editor
 */
class ReAbstractTextureEditor : public QWidget {

  Q_OBJECT

protected:

  ReTextureChannelDataModelPtr model;
  ReMaterial* material;

public:
  // Constructor: ReAbstractTextureEditor
  ReAbstractTextureEditor( QWidget* parent = 0 ) : QWidget( parent ) {
  };
  
  // Destructor: ReAbstractTextureEditor
 ~ReAbstractTextureEditor() {
  };

  virtual void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat) {
    model = _model;
    material = mat;
  } 

signals:
  //!   Emitted when one of the embedded textures is being edited. This is used
  //!   to "bubble" the signal up to the Texture Editor and cause the loading of 
  //!   the texture that is requested to be edited.
  void editTexture( ReTexturePtr tex, QString& breadcrumb );

  //!   Emitted when a compound texture creates a new sub-texture
  //!   This signal is meant to be received by the main Reality panel
  //!   which will then dispach it to the host-app side
  void makeNewTexture( const QString& objectID, 
                       const QString& materialName, 
                       const QString& channelID,
                       const QString& masterTextureName,
                       const QString& newTextureName,
                       const ReTextureType texType,
                       const ReTexture::ReTextureDataType texdataType );  

  //! Emitted when the user links an existing texture to a texture's channel
  void linkTexture( const QString& objectID,
                    const QString& materialID, 
                    const QString& channelID, 
                    const QString& textureName,
                    const QString& subTextureName );  

  //! Emitted when the user detaches an existing texture to a texture's channel
  void unlinkTexture( const QString& objectID,
                      const QString& materialID, 
                      const QString& textureName,
                      const QString& channelID );

  //! Emitted when the modifiers tab needs to be refreshed in response
  //! to the user making changes in the texture in the editor.
  void updateModifiers();

  //! Emitted when a texture is pasted, as in copy/paste.
  void replaceTexture( const QString& channelID,
                       const QString& jsonTextureData,
                       const QString& masterTextureName );

};


#endif