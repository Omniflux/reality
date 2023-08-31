/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef REMODIFIERS_H
#define REMODIFIERS_H

#include "ReMaterialEditor.h"
#include "RealityUI/ReTextureEditorAdapter.h"
#include "ui_meModifiers.h"


class ReModifiers: public ReMaterialEditor,
                   public Ui::meModifiers 
{
  Q_OBJECT

protected:
  // DisplaceableMaterial* material;
  bool inSetup;

  // Used for the timed editor
  QVariant updatedValue;
  QString updatedProperty;

  ReTextureEditorAdapterPtr bmAdapter, dmAdapter, klAdapter;

public:
  /**
   * ctor
   */
  ReModifiers( QWidget* parent = 0);

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

protected:


  //! Adjusts the value of the subdivision if a displacement map is
  //! specified and the subdivision value is set at 0
  void adjustSubdivision();
private slots:

  void updateBmStrength( int newVal );
  void updateBmNegative( double newVal );
  void updateBmPositive( double newVal );

  void updateDmStrength( int newVal );
  void updateDmNegative( double newVal );
  void updateDmPositive( double newVal );

  void updateSubdivisions( int newVal );
  void updateKeepSharpEdges( bool newVal );
  void updateUseMicrofacets( bool newVal );

  void setSmoothness( bool checked );
  // void fireUpdate();
  // void addNewTexture(const QString& ID, const ReTextureType texType);
  // void linkTextureHandler(const QString& ID, const QString& textureName);
  // void unlinkTextureHandler( const QString& ID );

  void updateHasLight(bool newVal);
  void updateLightGain(double newVal);

public slots:
  void refreshUI();

signals:
  void editTexture(ReTexturePtr tex, QString& breadcrumb);

  void makeNewTexture( const QString& objectID,
                       const QString& materialID, 
                       const QString& channelName, 
                       const QString& masterTextureName,
                       const QString& newTextureName,
                       const ReTextureType textureType,
                       const ReTexture::ReTextureDataType dataType );

};

#endif
