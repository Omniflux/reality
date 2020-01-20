/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_VELVET_EDITOR_H
#define RE_VELVET_EDITOR_H

#include <QWidget>
#include "ReVelvet.h"

#include "ReMaterialEditor.h"
#include "ReGeometryObject.h"
#include "ReTextureEditorAdapter.h"
#include "ui_meVelvet.h"

using namespace Reality;

/**
 This widget is used to edit a Velvet material.
 */
class ReVelvetEditor: public ReMaterialEditor,
                      public Ui::meVelvet 
{

  Q_OBJECT

private: 
  //! Avoid triggering the update of the material when calling the 
  //! <setMaterial()> method
  bool inSetup;

  //! Adapter to control the editing of the textures
  ReTextureEditorAdapterPtr KdAdapter;

public:
  /**
   * ctor
   */
  explicit ReVelvetEditor( QWidget* parent = 0);

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

private slots:
  void updateValue( double newVal );
};


#endif
