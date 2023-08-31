/**
 * The Reality texture manager.
 */

#include "RealityUI/ReImageMapManager.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include "actions/ReSetImageMapAction.h"
#include "textures/ReImageMap.h"


ReImageMapManager::ReImageMapManager(QWidget* parent) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

  fileName = "";
  dataType = ReTexture::color;
  rgbChannel = RGB_Mean;
  gain = 1.0;

  // Menu Actions
  newTextureAction     = QSharedPointer<QAction>(new QAction(tr("Select a new file"),this));
  showInFolderAction   = QSharedPointer<QAction>(new QAction(tr("Show in folder"),this));
  // Connections
  connect(newTextureAction.data(),    SIGNAL(triggered()), this, SLOT(selectNewTexture()));
  connect(showInFolderAction.data(),  SIGNAL(triggered()), this, SLOT(showTextureInFolder()));

  // Build the menu
  textureMenu.addAction(newTextureAction.data());
  textureMenu.addSeparator();
  textureMenu.addAction(showInFolderAction.data());
  btnChange->setMenu(&textureMenu);
}

QSize ReImageMapManager::sizeHint() const {
  return QSize(271, 310);
}

void ReImageMapManager::setLabel(QString newLabel) {
  label->setText(newLabel);
}

QString ReImageMapManager::getLabel() {
  return label->text();
}

void ReImageMapManager::setImageClass( const ReTexture::ReTextureDataType dtype ) {
  dataType = dtype;
}

void ReImageMapManager::setRgbChannel( const RGBChannel newVal ) {
  rgbChannel = newVal;
  updatePreview();
}

void ReImageMapManager::setGain( const float g ) {
  gain = g;
  updatePreview();
}

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


void setBrightness( QImage& img, const float gain ) {
  QColor pixel;
  qreal r,g,b;

  int w = img.width();
  int h = img.height();
  for(int x=0; x < w; x++){
    for(int y=0; y < h; y++){
      pixel = QColor(img.pixel(x,y));
      pixel.getRgbF(&r, &g, &b);
      r *= gain; 
      g *= gain;
      b *= gain;
      pixel.setRgbF(r, g, b);

      img.setPixel(x,y, pixel.rgba());
    }
  }
}

void ReImageMapManager::updatePreview() {
  if (fileName == "") {
    imPreview->clear();
    return;
  }

  QImage tmp;
  tmp.load(fileName);
  if (tmp.isNull()) {
    imPreview->clear();
    return;
  }
  // For successive uses of the image it's better
  // if we store the image at the size of the preview (240x240)
  // In this way we save memory and all the transformations take
  // much less time.
  int w,h;
  w = tmp.width();
  h = tmp.height();
  QSize imgSize(imPreview->width(), imPreview->height());
  QImage scaled = tmp.scaled(imgSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  // If the image is grayscale then we need to render the preview as such.
  // We can use either the median version of a grayscale obtained from one of the
  // channels.
  if ( dataType == ReTexture::numeric && !isNormalMap) {
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
  }
  // Store the image for efficiency
  if (gain != 1.0) {
    setBrightness(scaled, gain);
  }
  texPreview = QPixmap::fromImage(scaled);
  imPreview->setPixmap(texPreview);
  // Show the size of the original bitmap
  imFileSize->setText(QString("%1x%2").arg(w).arg(h));
}

void ReImageMapManager::setNormalMap( bool yesNo ) {
  isNormalMap = yesNo;
  updatePreview();
  update();
}

void ReImageMapManager::showTextureFileName() {
  lblTextureFileName->setText(QFileInfo(fileName).fileName());
  lblTextureFileName->setToolTip(fileName);  
}

void ReImageMapManager::setTextureFileName(QString newName) {
  fileName = newName;
  showTextureFileName();
  updatePreview();
}

QString ReImageMapManager::getTextureFileName() {
  return fileName;
}


void ReImageMapManager::selectNewTexture() {
  ReSetImageMapAction setImageMap(fileName);
  setImageMap.execute();

  if (setImageMap.hasNameChanged()) {
    QString fName = setImageMap.getFileName();
    isNormalMap = ReImageMap::checkIfNormalMap(fName);
    setTextureFileName(fName);
    emit mapHasChanged();
  }
}


void ReImageMapManager::showTextureInFolder() {
  showInFileManager(fileName);
}

void ReImageMapManager::showInFileManager( const QString &pathIn) {
    // Mac, Windows support folder or file.
#if defined(_WIN32)
    QStringList args; 
    args << "/select," << QDir::toNativeSeparators(pathIn);
    QProcess::startDetached("explorer", args);
#elif defined(__APPLE__)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(pathIn);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
#endif
}
