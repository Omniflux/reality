/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
 */


#ifndef RE_IMAGEMAP_TEXTURE_EDITOR_H
#define RE_IMAGEMAP_TEXTURE_EDITOR_H

#include <QUndoStack>

#include "ReTextureChannelDataModel.h"
#include "actions/ReTextureEditCommands.h"
#include "RealityUI/ReAbstractTextureEditor.h"
#include "textures/ReImageMap.h"
#include "ui_teImageMap.h"

#ifndef QT_CUSTOM_WIDGET
#include "RealityBase.h"
#endif


#define TC_COLOR      0
#define TC_GRAYSCALE  1
#define TC_FRESNEL    2

#define RGBCH_MEAN  0 
#define RGBCH_RED   1
#define RGBCH_GREEN 2
#define RGBCH_BLUE  3 

/**
 This widget is used to edit an ImageMap texture in the texture editor panel.
 */
class ReImageMapTextureEditor: public ReAbstractTextureEditor, 
                               public Ui::teImageMap 
{

  Q_OBJECT

private:
  bool updating;

public:

  /**
   * ctor
   */
  explicit ReImageMapTextureEditor( QWidget* parent = 0) : 
    ReAbstractTextureEditor(parent) 
  {
    setupUi(this);
    updating = false;
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

#ifndef QT_CUSTOM_WIDGET
    connect(imTextureName, SIGNAL(mapHasChanged()), this, SLOT(updateTexture()));
#endif

    // Handle the change of texture type
    connect(textureClass, SIGNAL(activated(const int)), 
            this,         SLOT(changeTextureClass(const int)));
    connect(textureMapping, SIGNAL(mappingChanged()), this, SLOT(updateMapping()));

    textureClass->addItem(tr("color"));
    textureClass->addItem(tr("grayscale"));
    textureClass->addItem(tr("fresnel"));

    connect(grayscaleChannel, SIGNAL(currentIndexChanged(int)), this, SLOT(updateGSChannel(int)) );
    connect(normalMap, SIGNAL(toggled(bool)), this, SLOT(updateNormalMap(bool)) );
  }

  QSize sizeHint() const {
    return QSize(280, 550);
  }

public slots:  

  void setDataModel( Reality::ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
    ReAbstractTextureEditor::setDataModel(_model, mat);
    refreshUI();    
    // Listen to when the model is changed
    connect(model.data(), SIGNAL(modelChanged()), this, SLOT(refreshUI()));
  }

private slots:
  void changeTextureClass(const int newVal) {
    grayscaleChannel->setEnabled(newVal == TC_GRAYSCALE);
    ReTexture::ReTextureDataType dType;
    switch( newVal ) {
      case TC_COLOR:
        dType = ReTexture::color;
        break;
      case TC_GRAYSCALE:
        dType = ReTexture::numeric;
        break;
      case TC_FRESNEL:
        dType = ReTexture::fresnel;
        break;
    }
    model->setNamedValue("textureDataType", dType );
    imTextureName->setImageClass(dType);
  }

  void updateTexture() {
    updating = true;
#ifndef QT_CUSTOM_WIDGET
    RealityBase::getUndoStack()->push(new IME_Command<QString>(
                                            model, 
                                            "fileName", 
                                            tr("Set image map"), 
                                            imTextureName->getTextureFileName()
                                          )
                                      );
#endif
    bool isNormalMap = model->getNamedValue("normalMap").toBool();
    normalMap->setChecked(isNormalMap);
    if (isNormalMap) {
      textureMapping->setGamma(1.0);
    }
    imTextureName->setNormalMap(isNormalMap);
    updateTextureClass(isNormalMap);
    updating = false;
    // If we switch from a bump map to a normal map we need to let the
    // Modifiers tab update
    emit updateModifiers();
  }
  
  //! Called when the mapping of the texture is changed. Generally used to
  //! update the texture preview in response to the user changing the gain
  //! parameters
  void updateMapping() {
    float newGain = textureMapping->getGain();
    if ( newGain != imTextureName->getGain() ) {
      imTextureName->setGain(newGain);
    }
  }

  void refreshUI() {
    if (updating) {
      return;
    }
    ImageMapPtr tex = model->getTexture().staticCast<ImageMap>();

    ReTexture::ReTextureDataType texClass = model->getTextureDataType();
    imTextureName->setTextureFileName(tex->getFileName());
    imTextureName->setImageClass(texClass);
    bool isNormalMap = tex->isNormalMap();
    int textureDataType = TC_COLOR;
    if (texClass == ReTexture::numeric) {
      textureDataType = TC_GRAYSCALE;
    }
    else if ( texClass == ReTexture::fresnel ) {
      textureDataType = TC_FRESNEL;
    }
    textureClass->setCurrentIndex(textureDataType);
    normalMap->setChecked(isNormalMap);
    updateTextureClass(isNormalMap);
    grayscaleChannel->setEnabled(textureDataType == TC_GRAYSCALE);

    if ( texClass == Reality::ReTexture::numeric && !isNormalMap ) {
      RGBChannel rgbCh = static_cast<RGBChannel>(tex->getRgbChannel());
      grayscaleChannel->setCurrentIndex(rgbCh == RGB_Mean  ? RGBCH_MEAN : 
                                        rgbCh == RGB_Red   ? RGBCH_RED : 
                                        rgbCh == RGB_Green ? RGBCH_GREEN : RGBCH_BLUE);
      imTextureName->setRgbChannel(rgbCh);
    }
    imTextureName->setNormalMap(isNormalMap);
    textureMapping->setModel(model);
    imTextureName->setGain(textureMapping->getGain());
  }

  void updateGSChannel( int newVal ) {
    RGBChannel newChannel = RGB_Mean;
    switch( newVal ) {
      case RGBCH_RED:
        newChannel = RGB_Red;
        break;
      case RGBCH_GREEN:
        newChannel = RGB_Green;
        break;
      case RGBCH_BLUE:
        newChannel = RGB_Blue;
        break;
    }
    model->setNamedValue("rgbChannel", newChannel);
    imTextureName->setRgbChannel(newChannel);
  }

  void updateTextureClass( const bool isNormalMapOn ) {
    if (isNormalMapOn) {
      textureClass->setCurrentIndex(TC_GRAYSCALE);
      textureClass->setEnabled(false);
      model->setNamedValue("textureDataType", ReTexture::numeric );
      model->setNamedValue("gamma", 1.0 );
    }
    else {
      textureClass->setEnabled(true);
    }
  }

  void updateNormalMap( bool onOff ) {
    if (updating) {
      return;
    }
    updateTextureClass(onOff);
    model->setNamedValue("normalMap", onOff);
    if (onOff) {
      model->setNamedValue("gamma", 1.0);
    }
    refreshUI();
    emit updateModifiers();
  }
};


#endif
