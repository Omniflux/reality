/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_COLOR_TEXTURE_EDITOR_H
#define RE_COLOR_TEXTURE_EDITOR_H

#include <algorithm>
#include <QColor>
#include <QSharedPointer>
#include <QUndoStack>

#include "RealityBase.h"
#include "actions/ReTextureEditCommands.h"
#include "RealityUI/ReAbstractTextureEditor.h"
#include "ui_teColor.h"

namespace Reality {
  class ReMaterial;
  class ReTextureChannelDataModel;
  typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
}


using namespace Reality;

// A list of colors, we have 5 elements per list
typedef QList<QColor> ReColorList;
// A preset is a map of colors keyed with a name for the preset (hair, skin, ...)
// Each preset has 5 ReColorList for a total of 25 colors
typedef QMap<QString, QList<ReColorList> > ReColorPresets;
typedef QMapIterator<QString, QList<ReColorList> > ReColorPresetsIterator;

/**
 This widget is used to edit a Color texture in the texture editor panel.
 */
class ReColorTextureEditor: public ReAbstractTextureEditor, 
                            public Ui::teColor {
  Q_OBJECT

private:
  bool updating;

  ReColorPresets colorPresets;
public:
  /**
   * ctor
   */
  explicit ReColorTextureEditor( QWidget* parent = 0) : 
    ReAbstractTextureEditor(parent) 
  {
    setupUi(this);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    updating = false;
    connect(teColorPicker, SIGNAL(colorChanged()), this, SLOT(updateTexture()));
    connect(presetSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(updateColorPreset(int)));

    initColorPresets();

    clrPreset1_1->installEventFilter(this);
    clrPreset1_2->installEventFilter(this);
    clrPreset1_3->installEventFilter(this);
    clrPreset1_4->installEventFilter(this);
    clrPreset1_5->installEventFilter(this);

    clrPreset2_1->installEventFilter(this);
    clrPreset2_2->installEventFilter(this);
    clrPreset2_3->installEventFilter(this);
    clrPreset2_4->installEventFilter(this);
    clrPreset2_5->installEventFilter(this);

    clrPreset3_1->installEventFilter(this);
    clrPreset3_2->installEventFilter(this);
    clrPreset3_3->installEventFilter(this);
    clrPreset3_4->installEventFilter(this);
    clrPreset3_5->installEventFilter(this);

    clrPreset4_1->installEventFilter(this);
    clrPreset4_2->installEventFilter(this);
    clrPreset4_3->installEventFilter(this);
    clrPreset4_4->installEventFilter(this);
    clrPreset4_5->installEventFilter(this);

    clrPreset5_1->installEventFilter(this);
    clrPreset5_2->installEventFilter(this);
    clrPreset5_3->installEventFilter(this);
    clrPreset5_4->installEventFilter(this);
    clrPreset5_5->installEventFilter(this);    
  }

  QSize sizeHint() const {
    return QSize(196, 244);    
  }

  void setDataModel( Reality::ReTextureChannelDataModelPtr _model, ReMaterial* mat );

protected:
  bool eventFilter(QObject* obj, QEvent* event);
  

private:

  void initColorPresets();

private slots:
  void updateTexture() {
    updating = true;
    // Check if the color texture is actually a BW color and if so
    // correct the color if it has been set to RGB
    if (model->getTextureDataType() == ReTexture::numeric) {
      QColor clr = teColorPicker->getColor();
      int r,g,b;
      clr.getRgb(&r,&g,&b);
      int maxRGB = std::max(r, std::max(g,b));
      int minRGB = std::min(r, std::min(g,b));
      int newRGB = (maxRGB + minRGB) / 2;
      teColorPicker->setColor(QColor(newRGB, newRGB, newRGB));
    }
    RealityBase::getUndoStack()->push(new ColorEdit_Command<QColor>(
                                            model, 
                                            "color", 
                                            tr("Set color"), 
                                            teColorPicker->getColor()
                                          )
                                      );    
    updating = false;
  }

  void updateColorPreset( int index );
  void loadPreset( const QString presetName );  
  void refreshUI();
};

#endif
