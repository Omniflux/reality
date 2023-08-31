/**
 *
 */

#ifndef RE_METAL_EDITOR_H
#define RE_METAL_EDITOR_H

#include "ReMaterialEditor.h"
#include "ReMetal.h"
#include "RealityUI/ReTextureEditorAdapter.h"
#include "ui_meMetal.h"

class QListWidgetItem;


/**
 This widget is used to edit a Metal material.
 */
class ReMetalEditor: public ReMaterialEditor,
                     public Ui::meMetal 
{
  Q_OBJECT

private: 
  // Avoid triggering the update of the material when calling the 
  // <setMaterial()> method
  bool inSetup;

  QMap< QString, ReMetal::MetalType > lutMetalNameToType;
  QMap< ReMetal::MetalType, QString > lutMetalTypeToName;

  void initLUT();

  QVariant updatedValue;
  QString updatedProperty;

  QSharedPointer<ReTextureEditorAdapter> teAdapter;
public:
  /**
   * ctor
   */
  explicit ReMetalEditor( QWidget* parent = 0);

  QSize sizeHint() const;

  void setData( ReMaterial* newMat, ReTextureChannelDataModelPtr tcm );

public slots:

  void updateHPolish(int newVal);
  void updateVPolish(int newVal);
  void updatePreset( QListWidgetItem* current, QListWidgetItem* previous );
  void updateCustomMetal( bool onOff );
  void updateAnisotropic( bool onOff );

};


#endif
