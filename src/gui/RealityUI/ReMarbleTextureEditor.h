/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MARBLE_NOISE_TEXTURE_EDITOR_H
#define RE_MARBLE_NOISE_TEXTURE_EDITOR_H


#include <QKeyEvent>

#include "textures/ReMarble.h"
#include "ReTextureChannelDataModel.h"
#include "ReAbstractTextureEditor.h"
#include "actions/ReTextureEditCommands.h"
#include "ReTimedEditor.h"
  

  #include "RealityBase.h"
  #include "ReMaterialPreview.h"



#include "ui_teMarble.h"

/*
 Class: ReMarbleTextureEditor

 This widget is used to edit a Marble texture in the texture editor panel.
 */

using namespace Reality;

class ReMarbleTextureEditor: public ReAbstractTextureEditor, 
                             public ReTimedEditor,
                             public Ui::teMarble 
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
  explicit ReMarbleTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const{
    return QSize(310, 580);
  }


  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );


private slots:
  void refreshUI();



public slots:
  
  void requestPreview();
  void updatePreview(QString matName, QString previewID,QImage* preview);
  void updateNoiseType( int noiseIndex );  
  void updateValue( double newVal );  
  void updateVeinQuality( bool onOff );
  void updateVeinWave( bool onOff );
  void updateNoiseHardness( bool onOff );
  


};

#endif
