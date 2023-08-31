/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_WOOD_NOISE_TEXTURE_EDITOR_H
#define RE_WOOD_NOISE_TEXTURE_EDITOR_H

#include <QSharedPointer>

#include "RealityUI/ReAbstractTextureEditor.h"
#include "RealityUI/ReTimedEditor.h"
#include "ui_teWood.h"


namespace Reality {
  class ReMaterial;
  class ReMaterialPreview;
  class ReTextureChannelDataModel;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
}


/*
 Class: ReWoodTextureEditor

 This widget is used to edit a Wood texture in the texture editor panel.
 */

using namespace Reality;

class ReWoodTextureEditor: public ReAbstractTextureEditor, 
                             public ReTimedEditor,
                             public Ui::teWood 
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
  explicit ReWoodTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const;


  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );


private slots:
  void refreshUI();



public slots:
  
  void requestPreview();
  void updatePreview(QString matName, QString previewID,QImage* preview);
  void updateNoiseType( int noiseIndex );  
  void updateWoodPattern( int newVal );
  void updateValue( double newVal );  
  void updateVeinWave( bool onOff );
  void updateNoiseHardness( bool onOff );
  // Decide if noise hardness needs to be enabled based 
  // on the wood pattern.
  void configureNoiseWidgets( int newVal );
  


};

#endif
