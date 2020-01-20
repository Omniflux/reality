/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MATTE_EDITOR_H
#define RE_MATTE_EDITOR_H

#include <QDebug>
#include <QWidget>
#include "ReMatte.h"

#include "ReMaterialEditor.h"
#include "ReGeometryObject.h"
#include "ui_meMatte.h"
#include "ReTextureEditorAdapter.h"

using namespace Reality;

/**
 This widget is used to edit a Matte material.
 */
class ReMatteEditor: public ReMaterialEditor,
                     public Ui::meMatte 
{

  Q_OBJECT

private: 
  //! Avoid triggering the update of the material when calling the 
  //! <setMaterial()> method
  bool inSetup;
  // Adapters to control the editing of the textures
  ReTextureEditorAdapterPtr KdAdapter,
                            KtAdapter;

protected:
  Reality::ReMatte* currentMaterial;                        

  void signalUpdate(QString valueName, QVariant value);

public:
  /**
   * ctor
   */
  explicit ReMatteEditor( QWidget* parent = 0);

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

private slots:

  void updateRoughness(double newVal);
  void updateTranslucency( bool onOff );
  void updateConserveEnergy( bool onOff );
/*
signals:

  void makeNewTexture(const QString& objectID,
                      const QString& materialID, 
                      const QString& channelName, 
                      const QString& masterTextureName, 
                      const QString& newTextureName, 
                      const ReTextureType textureType,
                      const ReTexture::ReTextureDataType dataType);
  */
};


#endif
