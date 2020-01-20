/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __RE_BRICKS_TEXTURE_EDITOR_H__
#define __RE_BRICKS_TEXTURE_EDITOR_H__


#include <QKeyEvent>

#include "ReTexture.h"
#include "ReTextureChannelDataModel.h"
#include "ReAbstractTextureEditor.h"
#include "actions/ReTextureEditCommands.h"
#include "ReTimedEditor.h"  
#include "RealityBase.h"
#include "ReMaterialPreview.h"

#include "ui_teBricks.h"


using namespace Reality;

/**
 This widget is used to edit a Bricks texture in the texture editor panel.
 */
class ReBricksTextureEditor: public ReAbstractTextureEditor, 
                             public ReTimedEditor,
                             public Ui::teBricks {
  Q_OBJECT

private:

  ReMaterialPreview* previewMaker;
  //! Flag used to avoid refreshing the UI while the UI is updating the model
  bool uiUpdating;

public:
  /**
   * ctor
   */
  explicit ReBricksTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const{
    return QSize(269, 545);
  }

  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );

private slots:
  void refreshUI();

public slots:
  
  void requestPreview();
  void updatePreview(QString matName, QString previewID,QImage* preview);
  void updateBrickType( int noiseIndex );  
  void updateValue( double newVal );  

};

#endif
