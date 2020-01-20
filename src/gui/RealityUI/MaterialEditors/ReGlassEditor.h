/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_GLASS_EDITOR_H
#define RE_GLASS_EDITOR_H

#include <QWidget>
#include "ReGlass.h"
#include "ReMaterialEditor.h"
#include "ReTextureEditorAdapter.h"

#include "ui_meGlass.h"

/**
 This widget is used to edit a Glass material.
 */

using namespace Reality;

class ReGlassEditor: public ReMaterialEditor, 
                     public Ui::meGlass  
{
  
  Q_OBJECT

private:
  ReGlass* currentMaterial;

  bool inSetup;

  void loadIORPresets();

  // Adapters to control the editing of the textures
  ReTextureEditorAdapterPtr KrAdapter,
                            KtAdapter;

public:
  /**
   * ctor
   */
  explicit ReGlassEditor( QWidget* parent = 0);
  QSize sizeHint() const;


  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );



private:
  //! Enable or disable widgets based on the type of glass selected
  void adjustEditor();
  void createVolume();

private slots:
  void selectIORPreset();
  void updateURoughness( double val );
  void updateVRoughness( double val );
  void updateThinFilmThickness( double val );
  void updateThinFilmIOR( double val );
  void updateGlassType();
  void updateIOR( double val );
  void updateCauchyB( double val );
  void updateClarity( double val );
  void updateDispersion( bool val );

  void signalUpdate(QString valueName, QVariant value);
  void updateAnisotropic( bool onOff );


};


#endif
