/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
 */

#ifndef RE_GLOSSY_EDITOR_H
#define RE_GLOSSY_EDITOR_H

#include <QWidget>
#include "ReMaterials.h"
#include "ReGlossy.h"
#include "ReMaterialEditor.h"
#include "ReGeometryObject.h"
#include "ReTimedEditor.h"
#include "ReTextureEditorAdapter.h"
#include "ui_meGlossy.h"

/**
 This widget is used to edit a Glossy material.
 */
class ReGlossyEditor: public ReMaterialEditor,
                      public Ui::meGlossy 
{

  Q_OBJECT

private: 
  // Avoid triggering the update of the material when calling the 
  // <setMaterial()> method
  bool inSetup;
  QVariant updatedValue;
  QString updatedProperty;

protected:
  // The adapters for editing the textures
  ReTextureEditorAdapterPtr KdAdapter,
                            KsAdapter,
                            KaAdapter,
                            KtAdapter,
                            KgAdapter;
public:
  /**
   * ctor
   */
  explicit ReGlossyEditor( QWidget* parent = 0 );

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

private slots:
  // void addNewTexture(const QString& ID, const ReTextureType texType);
  // void linkTextureHandler(const QString& ID, const QString& textureName);
  // void unlinkTextureHandler(const QString& ID);
  void updateTranslucency( bool newVal );
  void updateTopCoat( bool newVal );

public slots:

  void updateUGlossiness(int newVal);
  void updateVGlossiness(int newVal);
  void updateCoatThickness(double newVal);
  void updateMixLevel(double newVal);
  void updateSurfaceFuzz( bool newVal );
  void updateAnisotropic( bool onOff );
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
