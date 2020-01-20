/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.  

*/

#ifndef RE_3DMAPPING_EDITOR_H
#define RE_3DMAPPING_EDITOR_H


#include <QKeyEvent>
#include "ReTextureChannelDataModel.h"
#include "ReAbstractTextureEditor.h"
#include "actions/ReTextureEditCommands.h"
#include "ReTimedEditor.h"
  
#include "ui_te3DMapping.h"

using namespace Reality;

/**
 This widget is used to edit a the 3D mapping (scale and mapping mode) of
 a 3D texture. It's included in all the 3D texture editors as a sub-widget
 and it shares the same data model.
 */
class Re3DMappingEditor: public ReAbstractTextureEditor, 
                         public Ui::te3DMapping 
{
  Q_OBJECT

private:
  // Variable: uiUpdating
  //   Flag used to avoid refreshing the UI while the UI is updating the model
  bool uiUpdating;

public:
  /**
   * ctor
   */
  explicit Re3DMappingEditor( QWidget* parent = 0 );
  
  QSize sizeHint() const;

  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat );

private slots:
  void refreshUI();

public slots:
  
  void updateScale( double newVal );  
  void updateMapping( const QString& newVal );  

signals:
  void mappingChanged();
};

#endif
