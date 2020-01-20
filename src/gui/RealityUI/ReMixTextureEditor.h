/**
 *
 */

#ifndef RE_MIX_TEXTURE_EDITOR_H
#define RE_MIX_TEXTURE_EDITOR_H

#include <QTimer>

#include "ReMix.h"
#include "ReTextureChannelDataModel.h"
#include "ReAbstractTextureEditor.h"
#include "ReTimedEditor.h"
#include "ReTextureEditorAdapter.h"


  #include "RealityBase.h"
  #include "ReMaterialPreview.h"


#include "ui_teMix.h"


using namespace Reality;

/**
 * This widget is used to edit a Mix texture in the texture editor panel.
 */
class ReMixTextureEditor: public ReAbstractTextureEditor,
                          // public ReTimedEditor,
                          public Ui::teMix 
{
  Q_OBJECT

private:
  bool uiUpdating;
  ReTextureEditorAdapterPtr Tex1Adapter, Tex2Adapter,MixAdapter;

public:
  /**
   * ctor
   */
  explicit ReMixTextureEditor( QWidget* parent = 0 );
  QSize sizeHint() const;


  void setDataModel( Reality::ReTextureChannelDataModelPtr model, ReMaterial* mat );

private slots:
  void refreshUI();
  void updateMixAmount(double newVal);



};

#endif
