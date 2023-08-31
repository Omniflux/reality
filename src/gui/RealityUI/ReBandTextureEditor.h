/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_BAND_TEXTURE_EDITOR_H
#define RE_BAND_TEXTURE_EDITOR_H

#include "RealityUI/ReAbstractTextureEditor.h"
#include "RealityUI/ReTextureEditorAdapter.h"
#include "ui_teBand.h"

namespace Reality {
  enum ReTextureType;
  class ReMaterial;
  class ReTextureChannelDataModel;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
}


using namespace Reality;

/**
 This widget is used to edit an Band texture in the texture editor panel.
 */

class ReBandTextureEditor: public ReAbstractTextureEditor, 
                           public Ui::teBand 
{
  Q_OBJECT

private:
  // Variable: uiUpdating
  //   Flag used to avoid refreshing the UI while the UI is updating the model
  bool uiUpdating;

  // The adapters for editing the textures
  ReTextureEditorAdapterPtr tex1Adtp,tex2Adtp,tex3Adtp,tex4Adtp;
  ReTextureEditorAdapterPtr amountTextureAdp;

public:
  /**
   * ctor
   */
  explicit ReBandTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const;

  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );

private slots:
  void refreshUI();
  // void addNewTexture( const QString& ID, const ReTextureType texType );


public slots:
  void updateValue( double newVal );  

signals:
  //! Emitted when the user detaches a texture from a material's channel
  void unlinkTexture( const QString& objectID,
                      const QString& materialID, 
                      const QString& channelID );

  //! Emitted when the user links an existing texture to a material's channel
  void linkTexture( const QString& objectID,
                    const QString& materialID, 
                    const QString& channelID, 
                    const QString& textureName );

  //! Emitted when a component of a material has been edited
  void materialUpdated( QString objectID, 
                        QString materialName, 
                        QString propertyName, 
                        QVariant value );

  //! Emitted when the user adds a new texture to a material channel
  void makeNewTexture( const QString& objectID,
                       const QString& materialID, 
                       const QString& channelName, 
                       const ReTextureType textureType,
                       const ReTexture::ReTextureDataType dataType );

  void makeNewTexture( const QString& objectID,
                       const QString& materialID, 
                       const QString& channelName, 
                       const QString& masterTextureName,
                       const QString& newTextureName,
                       const ReTextureType textureType,
                       const ReTexture::ReTextureDataType dataType );

};

#endif
