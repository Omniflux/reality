/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_COLORMATH_TEXTURE_EDITOR_H
#define RE_COLORMATH_TEXTURE_EDITOR_H


#include "ReTexture.h" 
#include "ui_teColorMath.h"
#include "ReAbstractTextureEditor.h"
#include "ReTextureChannelDataModel.h"
#include "ReTextureEditorAdapter.h"
#include "RealityBase.h"
#include "ReMaterialPreview.h"


using namespace Reality;

/**
 * This widget is used to edit a ColorMath texture in the texture editor panel.
 */
class ReColorMathTextureEditor: public ReAbstractTextureEditor,
                                public Ui::teColorMath {
  Q_OBJECT

private:
  ReTextureEditorAdapterPtr Tex1Adapter, 
                            Tex2Adapter;

  //! Flag used to avoid sending signals during UI setup
  bool dontSignal;
public:
  /**
   * ctor
   */
  explicit ReColorMathTextureEditor( QWidget* parent = 0 );
  QSize sizeHint() const;

  void setDataModel( Reality::ReTextureChannelDataModelPtr _model, ReMaterial* mat );

private slots:
  void updateColor1();
  void updateColor2();
  void updateFunction(int newVal);
};

#endif
