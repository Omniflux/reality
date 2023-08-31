/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_IMAGEMAP_IMPORTER_H
#define QT_IMAGEMAP_IMPORTER_H

#include <QFileInfo>

#include "reality_lib_export.h"
#include "RealityBase.h"
#include "ReQtTextureImporter.h"
#include "textures/ReImageMap.h"


namespace Reality {

/**
  Class: ReQtImageMapExporter
 */

class REALITY_LIB_EXPORT ReQtImageMapImporter : public ReQtTextureImporter {
private:
  bool libPathsLoaded;
  QStringList libPaths;

  //! We store the last library path that matched a file. Chances are that many textures 
  //! come from the same path so it makes sense to test them against a matched path 
  //! bofore trying all the other paths in the library
  QString lastLibraryPath;

public:

  ReQtImageMapImporter() {
    libPathsLoaded = false;
  }

  QString resolveRuntimePath( const QString& filePath ) {
    if ( !libPathsLoaded ) {
      libPaths = RealityBase::getLibraryPaths();
      libPathsLoaded = true;
    }
    // Is this a path relative to one of the Runtimes?
    if ((filePath.length() > 0) && (filePath[0] == '@')) {
      // First check the last path used
      if (lastLibraryPath != "") {
        QFileInfo fileTest( QString("%1/%2").arg(lastLibraryPath).arg(filePath.mid(1)));
        if (fileTest.exists()) {
          return fileTest.absoluteFilePath();
        }
      }
      // Not found there so let's scan the paths in the library
      QStringListIterator i(libPaths);
      while( i.hasNext() ) {
        QString directory = i.next();
        QFileInfo fileTest( QString("%1/%2").arg(directory).arg(filePath.mid(1)));
        if (fileTest.exists()) {
          lastLibraryPath = directory;
          return fileTest.absoluteFilePath();
        }
      }
    }
    return filePath;
  }

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ImageMapPtr tex = baseTex.staticCast<ImageMap>();
    restoreBasicProperties(tex, data);

    tex->setFileName(resolveRuntimePath(data.value("fileName").toString()));
    tex->setGain(data.value("gain").toFloat());
    tex->setGamma(data.value("gamma").toFloat());
    tex->setRgbChannel(static_cast<RGBChannel>(data.value("rgbChannel").toInt()));
    tex->setNormalMap(data.value("normalMap", false).toBool());
    restore2DMapping(tex,data);    
  };
};

}
#endif
