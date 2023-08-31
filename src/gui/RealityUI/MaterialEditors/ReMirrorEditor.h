/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MIRROR_EDITOR_H
#define RE_MIRROR_EDITOR_H

#include "ReMaterialEditor.h"
#include "RealityUI/ReTextureEditorAdapter.h"
#include "ui_meMirror.h"


using namespace Reality;

/**
 This widget is used to edit a Mirror material.
 */
class ReMirrorEditor: public ReMaterialEditor,
                      public Ui::meMirror 
{

  Q_OBJECT

private: 
  //! Avoid triggering the update of the material when calling the 
  //! setMaterial() method
  bool inSetup;

  //! Adapter to control the editing of the texture
  ReTextureEditorAdapterPtr KrAdapter;

public:
  /**
   * Constructor
   */
  explicit ReMirrorEditor( QWidget* parent = 0);

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

private slots:
  void updateValue( double newVal );
};


#endif
