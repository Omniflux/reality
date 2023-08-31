/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __RE_CHECKERS_TEXTURE_EDITOR_H__
#define __RE_CHECKERS_TEXTURE_EDITOR_H__

#include <QSharedPointer>

#include "RealityUI/ReAbstractTextureEditor.h"
#include "RealityUI/ReTimedEditor.h"
#include "ui_teCheckers.h"

namespace Reality {
  class ReMaterial;
  class ReMaterialPreview;
  class ReTextureChannelDataModel;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
}


using namespace Reality;

/**
 This widget is used to edit an Checkers texture in the texture editor panel.
 */
class ReCheckersTextureEditor: public ReAbstractTextureEditor, 
                          public ReTimedEditor,
                          public Ui::teCheckers {
  Q_OBJECT

private:

  ReMaterialPreview* previewMaker;

  //! Flag used to avoid refreshing the UI while the UI is updating the model
  bool uiUpdating;

public:
  /**
   * ctor
   */
  explicit ReCheckersTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const;

  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );

private slots:
  void refreshUI();

public slots:
  
  void requestPreview();
  void updatePreview( QString matName, QString previewID, QImage* preview );

};

#endif
