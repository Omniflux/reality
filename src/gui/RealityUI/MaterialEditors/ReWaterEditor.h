/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_WATER_EDITOR_H
#define RE_WATER_EDITOR_H

#include <QWidget>
#include "ReWater.h"
#include "ReMaterialEditor.h"
#include "ReGeometryObject.h"
#include "ReTimedEditor.h"
#include "ui_meWater.h"

using namespace Reality;

using namespace Reality;

/**
 * This widget is used to edit a Water material.
 */
class ReWaterEditor: public ReMaterialEditor,
                     public Ui::meWater 
{

  Q_OBJECT

private: 
  // Avoid triggering the update of the material when calling the 
  // <setMaterial()> method
  bool inSetup;
  QVariant updatedValue;
  QString updatedProperty;
  
public:
  /**
   * ctor
   */
  explicit ReWaterEditor( QWidget* parent = 0 );

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

private slots:
  void updateKt();
  void updateIOR(double newVal);
  void updateRippleType(int);

public slots:
  void updateRipples(double newVal);
  void updateClarity(double newVal);

signals:
  void refreshModifiers();
  void refreshVolumes();
  void textureUpdated(ReTexturePtr tex, const QString& valueName, const QVariant& value );
};


#endif
