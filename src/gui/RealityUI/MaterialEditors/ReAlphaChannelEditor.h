/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
 */

#ifndef RE_ALPHACHANNEL_EDITOR_H
#define RE_ALPHACHANNEL_EDITOR_H

#include <QWidget>
#include "ReMaterialEditor.h"
#include "ReTimedEditor.h"
#include "ReTextureEditorAdapter.h"
#include "ui_meAlpha.h"

/**
 This widget is used to edit the Alpha Channel of a material.
 */
class ReAlphaChannelEditor: public ReMaterialEditor,
                            public Ui::meAlphaChannel
{

  Q_OBJECT

private: 
  // Avoid triggering the update of the material when calling the 
  // setData() method
  bool inSetup;

protected:
  // The adapter for editing the textures
  ReTextureEditorAdapterPtr alphaAdapter;

public:
  /**
   * ctor
   */
  explicit ReAlphaChannelEditor( QWidget* parent = 0 );

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

private slots:
  //! Called when the user changes the value for the alpha channel
  void updateAlphaValue(double newVal);
signals:

  void makeNewTexture(const QString& objectID,
                      const QString& materialID, 
                      const QString& channelName, 
                      const QString& masterTextureName, 
                      const QString& newTextureName, 
                      const ReTextureType textureType,
                      const ReTexture::ReTextureDataType dataType);
};


#endif
