/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_MATH_TEXTURE_EDITOR_H
#define RE_MATH_TEXTURE_EDITOR_H


#include "ReTexture.h" 
#include "ui_teMath.h"
#include "ReAbstractTextureEditor.h"
#include "ReTextureChannelDataModel.h"
#include "ReTextureEditorAdapter.h"


using namespace Reality;

/**
 This widget is used to edit a Math texture in the texture editor panel.
 */
class ReMathTextureEditor: public ReAbstractTextureEditor,
                           public Ui::teMath 
{
  Q_OBJECT

private:
  bool inSetup;

  ReTextureEditorAdapterPtr Tex1Adapter, 
                            Tex2Adapter;

public:
  /**
   * ctor
   */
  explicit ReMathTextureEditor( QWidget* parent = 0);
  QSize sizeHint() const;

  void setDataModel( Reality::ReTextureChannelDataModelPtr _model, ReMaterial* mat );

private slots:
  void updateAmount1(double newVal);
  void updateAmount2(double newVal);
  void updateFunction(int newVal);

};

#endif
