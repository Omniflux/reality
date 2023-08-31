/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
*/

#include "RealityUI/ReGrayscaleTextureEditor.h"

#include <QImage>

#include "ReGeometryObject.h"
#include "ReTextureCreator.h"
#include "textures/ReGrayscale.h"


ReGrayscaleTextureEditor::ReGrayscaleTextureEditor( QWidget* parent ) : 
  ReAbstractTextureEditor(parent)
{
  setupUi(this);
  updating = false;
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  // Editing of the texture...
  connect(baseTexture, SIGNAL(editTexture( ReTexturePtr, QString&)), 
          this,    SIGNAL(editTexture( ReTexturePtr, QString&)));

  // RGB channel selector
  connect(cbRgbChannel, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateRGBChannel(int)));

  // Create new texture signal
  connect(baseTexture, SIGNAL(addNewTexture(const QString&, const ReTextureType)), 
          this,    SLOT(addNewTexture(const QString&, const ReTextureType)));

  // Link an existing texture
  connect(baseTexture, SIGNAL(linkTexture(const QString&, const QString&)), 
          this,    SLOT(linkTextureHandler(const QString&, const QString&)));

  connect(baseTexture, SIGNAL(unlinkTexture(const QString&)), 
          this, SLOT(unlinkTextureHandler(const QString&)));

  baseTexture->setDeleteEnabled(true);
}

QSize ReGrayscaleTextureEditor::sizeHint() const {
  return QSize(310,380);
}

void ReGrayscaleTextureEditor::updatePreview() {
  QImage tmp;
  auto imTex = model->getNamedValue("texture").value<ReTexturePtr>();
  if (imTex.isNull()) {
    return;
  }
  if (imTex->getType() != TexImageMap) {
    preview->clear();
    return;
  }
  QString fileName = imTex->getNamedValue("fileName").toString();
  if (fileName == "") {
    return;
  }
  tmp.load(fileName);
  if (tmp.isNull()) {
    return;
  }
  // For successive uses of the image it's better
  // if we store the image at the size of the preview (240x240)
  // In this way we save memory and all the transformations take
  // much less time.
  int w,h;
  w = tmp.width();
  h = tmp.height();
  QSize imgSize(preview->width(), preview->height());
  QImage scaled = tmp.scaled(imgSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  int width = scaled.width();
  int height = scaled.height();
  // Pointer to the function used to transform the pixels.
  // This approach saves us thousands of "if" tests in the
  // conversion loop.
  void (*convertColor)(QRgb&);
  switch( rgbChannel ) {
    case RGB_Mean:
      convertColor = &setColorToGray;
      break;
    case RGB_Red:
      convertColor = &setColorToRed;
      break;
    case RGB_Green:
      convertColor = &setColorToGreen;
      break;
    case RGB_Blue:
      convertColor = &setColorToBlue;
      break;
  }
  // Convert the pixels
  for (int i = 0; i < width; ++i) {
    for (int l = 0; l < height; ++l) {
      QRgb clr = scaled.pixel(i, l);
      convertColor(clr);
      scaled.setPixel(i, l, clr);
    }
  }
  
  preview->setPixmap(QPixmap::fromImage(scaled));
}

void ReGrayscaleTextureEditor::refreshUI() {
  if (updating) {
    return;
  }
  ReTexturePtr tex = model->getTexture()->getNamedValue("texture").value<ReTexturePtr>();
  if (tex.isNull()) {
    return;
  }
  baseTexture->setTexture(tex, material);
  cbRgbChannel->setCurrentIndex(model->getNamedValue("rgbChannel").toInt());
  if (tex->getType() == TexImageMap) {
    textureMapping->setModel(model);
  }
  updatePreview();
}

void ReGrayscaleTextureEditor::updateRGBChannel( int channelNo ) {
  switch(channelNo) {
    case 0:
      rgbChannel = RGB_Red;
      break;
    case 1:
      rgbChannel = RGB_Green;
      break;
    case 2:
      rgbChannel = RGB_Blue;
      break;
  }
  model->setNamedValue("rgbChannel", rgbChannel);
  updatePreview();
}


void ReGrayscaleTextureEditor::unlinkTextureHandler( const QString& ID ) {
  auto tex = model->getTexture().staticCast<ReGrayscale>();
  tex->deleteChannelTexture(ID);
  emit unlinkTexture( material->getParent()->getInternalName(), 
                      material->getName(), 
                      tex->getName(),
                      "tex" );

}

void ReGrayscaleTextureEditor::linkTextureHandler( const QString& ID, 
                                                   const QString& textureName ) 
{
  auto tex = model->getTexture().staticCast<ReGrayscale>();
  ReTexturePtr linkedTex = material->getTexture(textureName);
  QVariant v;
  v.setValue<ReTexturePtr>(linkedTex);
  model->setNamedValue("texture", v);
  refreshUI();
  emit linkTexture( material->getParent()->getInternalName(), 
                    material->getName(), 
                    ID,
                    tex->getName(),
                    textureName );

}

void ReGrayscaleTextureEditor::addNewTexture( const QString& ID, 
                                              const ReTextureType texType ) 
{
  ReTexturePtr newTex = ReTexturePtr(
    ReTextureCreator::createTexture(
      QString("%1_%2").arg(model->getNamedValue("name").toString()).arg(ID),
      texType,
      material,
      model->getTextureDataType()
    )
  );
  material->addTextureToCatalog(newTex);
  model->getTexture().staticCast<ReGrayscale>()->setChannel("tex", newTex);

  emit makeNewTexture( material->getParent()->getInternalName(), 
                       material->getName(), 
                       "tex",
                       model->getNamedValue("name").toString(),
                       newTex->getName(),
                       texType,
                       newTex->getDataType());
}