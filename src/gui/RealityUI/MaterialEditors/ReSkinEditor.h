/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_SKIN_EDITOR_H
#define RE_SKIN_EDITOR_H

#include <QWidget>
#include "ReSkin.h"
#include "ReMaterialEditor.h"
#include "ReTimedEditor.h"
#include "ReTextureEditorAdapter.h"
#include "ReMaterialPreview.h"

#include "ui_meSkin.h"

/**
 \s widget is used to edit a Skin material.
 */
class ReSkinEditor: public ReMaterialEditor, 
                    public Ui::meSkin  
{
  
  Q_OBJECT

private:
  //! Avoid triggering the update of the material when calling the 
  //! <setMaterial()> method
  bool inSetup;
  QVariant updatedValue;
  QString updatedProperty;
  // Adapters to control the editing of the textures
  ReTextureEditorAdapterPtr KdAdapter,
                            KsAdapter,
                            KgAdapter,
                            KhmAdapter,
                            KaAdapter;


  //! Pointer to the material preview maker use to get the preview
  //! of the hair mask
  ReMaterialPreview* previewMaker;

  //! Connect to the material preview thread
  void connectToPreviewMaker();

  //! Updates the widgets that show the volume data for SSS.
  void updateSSSVolumeInfo();

public:
  /**
   * ctor
   */
  explicit ReSkinEditor( QWidget* parent = 0);

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

signals:
  //! Emitted when the volume for the material has been changed because the user
  //! has enabled/disabled SSS.
  void materialVolumeChanged( const QString& volName, bool onOff);
/*
  // Just to silence Qt about this signal not implemented. It's monitored by 
  // realitypanel.cpp
  void makeNewTexture(const QString& objectID,
                      const QString& materialID, 
                      const QString& channelName, 
                      const QString& masterTextureName, 
                      const QString& newTextureName, 
                      const ReTextureType textureType,
                      const ReTexture::ReTextureDataType dataType);
                      */

private slots:
  // void addNewTexture(const QString& ID, const ReTextureType texType);
  // void linkTextureHandler(const QString& ID, const QString& textureName);
  // void unlinkTextureHandler(const QString& ID);
  void updateSurfaceColor();
  void updateInteriorColor();
  void updateAbsorptionScale(double);
  void updateClarityAtDepth( double newVal );
  
  void updateUGlossiness(int newVal);
  void updateVGlossiness(int newVal);
  void updateSurfaceFuzz( bool newVal );
  void updateFresnel( int newVal );
  void updateScatteringScale( double newVal );
  void updateSSS( bool newVal );
  void updateCoat( bool onOff );
  void updateCoatThickness( double thickness );
  void updateHairMask( bool newVal );
  void updateHmGain( double newVal );
  void updateAnisotropic( bool onOff );  

  //! Responds to the material preview generator
  void updatePreview(QString matName, QString previewID, QImage* preview);
  void refreshHairMaskPreview();


};


#endif
