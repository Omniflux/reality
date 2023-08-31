/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_GRAYSCALE_TEXTURE_EDITO_H
#define RE_GRAYSCALE_TEXTURE_EDITO_H

#include "ReMaterial.h"
#include "ReTextureChannelDataModel.h"
#include "RealityUI/ReAbstractTextureEditor.h"
#include "ui_teGrayscale.h"

namespace Reality {
  enum ReTextureType;
  enum RGBChannel;
}


using namespace Reality;

// Color conversion functions used in the conversion of the preview
// to grayscale
inline void setColorToRed( QRgb& clr ) {
  int r = qRed(clr);
  clr = qRgb( r , r, r);
}

inline void setColorToGreen( QRgb& clr ) {
  int g = qGreen(clr);
  clr = qRgb( g , g, g);
}

inline void setColorToBlue( QRgb& clr ) {
  int b = qBlue(clr);
  clr = qRgb( b , b, b);
}

inline void setColorToGray( QRgb& clr ) {
  int g = qGray(clr);
  clr = qRgb(g,g,g);
}


class ReGrayscaleTextureEditor: public ReAbstractTextureEditor,
                                public Ui::teGrayscale {

  Q_OBJECT

private:
  bool updating;
  RGBChannel rgbChannel;

public:

  /**
   * ctor
   */
  explicit ReGrayscaleTextureEditor( QWidget* parent = 0);

  QSize sizeHint() const;

private:  

  void updatePreview();

private slots:
  void updateRGBChannel( int channelNo );
  void refreshUI();
  void linkTextureHandler( const QString& ID, const QString& textureName );
  void unlinkTextureHandler( const QString& ID );

  void addNewTexture( const QString& ID, const ReTextureType texType );

public slots:

  void setDataModel( ReTextureChannelDataModelPtr _model, ReMaterial* mat ) {
    ReAbstractTextureEditor::setDataModel(_model, mat);
    refreshUI();
  }

};

#endif
