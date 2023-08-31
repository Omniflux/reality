/**
 *
 */

#ifndef RE_MIX_TEXTURE_EDITOR_H
#define RE_MIX_TEXTURE_EDITOR_H

#include <QSharedPointer>
#include <QTimer>

#include "RealityUI/ReAbstractTextureEditor.h"
#include "ui_teMix.h"

namespace Reality {
  class ReMaterial;
  class ReTextureChannelDataModel;
  class ReTextureEditorAdapter;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
  typedef QSharedPointer<ReTextureEditorAdapter> ReTextureEditorAdapterPtr;
}


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
