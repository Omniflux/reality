/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE2DTEXTUREMAPPING_H
#define RE2DTEXTUREMAPPING_H


#include <QWidget>

#include "textures/Re2DTexture.h"
#include "ReTextureChannelDataModel.h"
#include "ui_tex2DMapping.h"

using namespace Reality;

/**
 * A widget that shows the parameters for the mapping of the texture. These
 * include: U Tile, V Tile, Gain, U&V Offset, Gamma and 
 * UV/Cylindrical/Spherical/Planar mapping
 */
class Re2DTextureMapping: public QWidget, public Ui::tex2DMapping {
  Q_OBJECT

protected:
  ReTextureChannelDataModelPtr model;

  bool inUpdate;
public:
  /**
   * ctor
   */
  explicit Re2DTextureMapping( QWidget* parent = 0);   
  QSize sizeHint() const;

public slots:

  void setModel( ReTextureChannelDataModelPtr newModel );
  /**
  * getUTile
  */
  float getUTile() {
    return uTile->value();
  };
  /**
  * setUTile
  */
  void setUTile( float newVal ) {
    uTile->setValue(newVal);
  };
  /**
   * getVTile
   */
  float getVTile() {
    return vTile->value();
  };
  /**
   * setVTile
   */
  void setVTile( float newVal ) {
    vTile->setValue(newVal);
  };

  /**
   * getUOffset
   */
  float getUOffset() {
    return uOffset->value();
  };
  /**
   * setUOffset
   */
  void setUOffset( float newVal ) {
    uOffset->setValue(newVal);
  };
  /**
   * getVOffset
   */
  float getVOffset() {
    return vOffset->value();
  };
  /**
   * setVOffset
   */
  void setVOffset( float newVal ) {
    vOffset->setValue(newVal);
  };
  /**
   * getGamma
   */
  float getGamma() {
    return gamma->value();
  };
  /**
   * setGamma
   */
  void setGamma( float newVal ) {
    gamma->setValue(newVal);
  };
  /**
   * getGain
   */
  float getGain() {
    return gain->value();
  };
  /**
   * setGain
   */
  void setGain( float newVal ) {
    gain->setValue(newVal);
  };

  void updateValue( double newVal );
  void updateValue( int newVal );

signals:  
  void mappingChanged();
};


#endif
