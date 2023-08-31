/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_DISTORTED_NOISE_TEXTURE_EDITOR_H
#define RE_DISTORTED_NOISE_TEXTURE_EDITOR_H

#include <QSharedPointer>

#include "RealityUI/ReAbstractTextureEditor.h"
#include "RealityUI/ReTimedEditor.h"
#include "ui_teDistortedNoise.h"

namespace Reality {
  class ReMaterial;
  class ReMaterialPreview;
  class ReTextureChannelDataModel;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
}


/*
 Class: ReDistortedNoiseTextureEditor

 This widget is used to edit a DistortedNoise texture in the texture editor panel.
 */

using namespace Reality;

class ReDistortedNoiseTextureEditor: public ReAbstractTextureEditor, 
                                     public ReTimedEditor,
                                     public Ui::teDistortedNoise 
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
  explicit ReDistortedNoiseTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const;


  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );


private slots:
  void refreshUI();



public slots:
  
  void requestPreview();
  void updatePreview(QString matName, QString previewID, QImage* preview);
  void updateNoiseType( int noiseIndex );  
  void updateDistortionType( int noiseIndex );  
  void updateValue( double newVal );  



};

#endif
