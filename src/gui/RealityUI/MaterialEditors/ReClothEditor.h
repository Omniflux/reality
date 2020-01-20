/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_CLOTH_EDITOR_H
#define RE_CLOTH_EDITOR_H

#include <QWidget>
#include "ReCloth.h"

#include "ReMaterialEditor.h"
#include "ReGeometryObject.h"
#include "ui_meCloth.h"
#include "ReTextureEditorAdapter.h"

using namespace Reality;

/**
 This widget is used to edit a Cloth material.
 */
class ReClothEditor: public ReMaterialEditor,
                     public Ui::meCloth 
{

  Q_OBJECT

private: 
  //! Avoid triggering the update of the material when calling the 
  //! <setMaterial()> method
  bool inSetup;

  // Adapters to control the editing of the textures
  ReTextureEditorAdapterPtr warpKdAdapter,
                            weftKdAdapter,
                            warpKsAdapter,
                            weftKsAdapter;

public:
  /**
   * ctor
   */
  explicit ReClothEditor( QWidget* parent = 0);

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

private slots:
  void updateValue( double newVal );
  void updatePreset();
};


#endif
