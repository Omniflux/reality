/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_FBM_TEXTURE_EDITOR_H
#define RE_FBM_TEXTURE_EDITOR_H


#include <QKeyEvent>

#include "textures/ReFBM.h"
#include "ReTextureChannelDataModel.h"
#include "ReAbstractTextureEditor.h"
#include "actions/ReTextureEditCommands.h"
#include "ReTimedEditor.h"
  

  #include "RealityBase.h"
  #include "ReMaterialPreview.h"



#include "ui_teFBM.h"


using namespace Reality;

/**
 This widget is used to edit an FBM texture in the texture editor panel.
 */
class ReFBMTextureEditor: public ReAbstractTextureEditor, 
                          public ReTimedEditor,
                          public Ui::teFBM 
{
  Q_OBJECT

private:


  ReMaterialPreview* previewMaker;


  // Variable: uiUpdating
  //   Flag used to avoid refreshing the UI while the UI is updating the model
  bool uiUpdating;

public:
  /**
   * ctor
   */
  explicit ReFBMTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const;


  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );


private slots:
  void refreshUI();



public slots:
  
  void requestPreview();
  void updatePreview(QString matName, QString previewID,QImage* preview);
  void updateValue( double newVal );  
  void updateValue( int newVal );  
  void updateWrinkled( bool );


};

#endif
