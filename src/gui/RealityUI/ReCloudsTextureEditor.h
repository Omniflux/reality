/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_CLOUDS_TEXTURE_EDITOR_H
#define RE_CLOUDS_TEXTURE_EDITOR_H

#include <QSharedPointer>

#include "RealityUI/ReAbstractTextureEditor.h"
#include "RealityUI/ReTimedEditor.h"
#include "ui_teClouds.h"

namespace Reality {
  class ReMaterial;
  class ReMaterialPreview;
  class ReTextureChannelDataModel;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
}


using namespace Reality;

/**
 This widget is used to edit a Clouds texture in the texture editor panel.
 */
class ReCloudsTextureEditor: public ReAbstractTextureEditor, 
                             public ReTimedEditor,
                             public Ui::teClouds 
{
  Q_OBJECT

private:

  ReMaterialPreview* previewMaker;

  //! Flag used to avoid refreshing the UI while the UI is updating the model
  bool uiUpdating;

public:
  /**
   * ctor
   */
  explicit ReCloudsTextureEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const;

  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );

private slots:
  void refreshUI();

private:

  void setNoisePattern( int noiseIndex );

public slots:
  
  void requestPreview();
  void updatePreview(QString matName, QString previewID, QImage* preview);
  void updateNoiseType( int noiseIndex );  
  void updateNoiseSize( double newVal );
  void updateNoiseDepth( int newVal );
  void updateBrightness( double newVal );
  void updateContrast( double newVal );
  void setHardSoftNoise( bool );

};

#endif
