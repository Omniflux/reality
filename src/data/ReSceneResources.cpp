/*
 *  ReSceneResources.cpp
 *
 *  Created by Paolo H Ciccone on 6/19/11.
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 *
 */

#include "ReSceneResources.h"

#include <QImageReader>
#include <QImageWriter>


namespace Reality {

#define RE_SCENE_RESOURCE_DIR "Resources"
#define RE_SCENE_OBJECTS      "objects"
#define RE_SCENE_TEXTURES     "textures"

ReSceneResources* ReSceneResources::instance = NULL;

ReSceneResources* ReSceneResources::getInstance() {
  if (!instance) {
    instance = new ReSceneResources();
  }
  return instance;
}

ReSceneResources::~ReSceneResources() {
  if (instance) {
    delete instance;
  }
}


/**
 * Method used to set the base location from where the collection directory will be created
 * The idea here is to point to the .lxo or .lxs file and create a Resources directory at the same level
 * so that all the texture paths in the .lxo file will be relative therefore making the operation of 
 * moving the Lux scene to another location a trivial copy.
 */
void ReSceneResources::setSceneName(const QString _sceneFileName) { 
  sceneFileName = _sceneFileName;
  QFileInfo geometryFileInfo(sceneFileName);  // The .lxo or .lxs file
  
  // Get the absolute path for the directory to be created
  sceneDir = geometryFileInfo.absolutePath();
  // Set the name for the Resources directory
  resourceDirName = geometryFileInfo.baseName() + "-" + RE_SCENE_RESOURCE_DIR;
  sceneResourceInfo.setFile(geometryFileInfo.absolutePath() + "/" + resourceDirName);
}

QString ReSceneResources::getSceneName() const {
  return(this->sceneFileName);
}

QString ReSceneResources::getResourcePath() {
  return(sceneResourceInfo.absoluteFilePath());
}

QString ReSceneResources::getResourceDirName() {
  return(resourceDirName);
}

void ReSceneResources::initialize() {
  if (sceneFileName.isEmpty()) {
    return;
  }

  QString resDirPath = sceneResourceInfo.absoluteFilePath();
  QDir resourceDir;
  
  if (!sceneResourceInfo.exists()) {
    if ( !resourceDir.mkdir(resDirPath) ) {
      RE_LOG_DEBUG() << QString(
                          "Error: could not create directory \"%1\" for Reality scene \"%2\""
                        )
                        .arg((const char*)resDirPath.toUtf8())
                        .arg((const char*)sceneFileName.toUtf8())
                        .toStdString();
      return;
    }
  };
  // Create the directory to store the PLY objects
  objectsPath = QString("%1/%2").arg(resDirPath).arg(RE_SCENE_OBJECTS);
  QDir objectsDir(objectsPath);
  if (!objectsDir.exists()) {
    objectsDir.mkdir(objectsPath);
  }
  else {
    // Remove the old PLYs
    QStringList oldPlys = objectsDir.entryList(QStringList("*.ply"));
    foreach( QString onePly, oldPlys ) {
      objectsDir.remove(onePly);
    }
    // Remove the old .hair files
    QStringList oldHairs = objectsDir.entryList(QStringList("*.hair"));
    foreach( QString oneHair, oldHairs ) {
      objectsDir.remove(oneHair);
    }
  };
  texturesPath = QString("%1/%2").arg(resDirPath).arg(RE_SCENE_TEXTURES);
  QDir texturesDir(texturesPath);
  if (!texturesDir.exists()) {
    texturesDir.mkdir(texturesPath);
  }
  textureSet.clear();
  initialized = true;
}


QString ReSceneResources::getObjectsPath()  {
  if (!initialized) {
    initialize();
  }
  return(this->objectsPath);
};

QString ReSceneResources::getTexturesPath()  {
  if (!initialized) {
    initialize();
  }
  return(this->texturesPath);  
}

/**
 * Method for removing the .ocl files form the kernel_cache directory created by SLG2
 * While kernel caches are supposed to speed things up, many times SLG crashes at startup
 * because of the presence of these files. Removing them ensures a clean start.
 */
void ReSceneResources::emptySLGKernelCache( const QString dirName ) {
  QDir dir(dirName);
  dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

  QFileInfoList list = dir.entryInfoList();
  for (int i = 0; i < list.size(); ++i) {
     QFileInfo fileInfo = list.at(i);
     if (fileInfo.isDir()) {
       emptySLGKernelCache(fileInfo.absoluteFilePath());
     }
     else if(fileInfo.isFile() && fileInfo.suffix() == "ocl" ) {
       dir.remove(fileInfo.fileName());
     }
  }
};

QString ReSceneResources::collectTexture( const QString fileName, 
                                          const ReTextureSize targetSize  ) 
{
  if (!initialized) {
    initialize();
  }
  // If after initialization the scene file name is still blank then 
  // the initialization is not concluded and so we need to stop.
  // This is usually happening when preparing the material preview while the
  // texture collection is enabled.
  if (!initialized) {
    return fileName;
  }

  if (fileName.isEmpty()) {
    return "";
  }
  // Don't repeat the operation if it was done before. Return the name used for the conversion
  if (textureSet.contains(fileName)) {
    return(textureSet[fileName]);
  };
  
  QFileInfo finfo(fileName);
  QString filePathStr = QString("%1/%2").arg(texturesPath).arg(finfo.dir().dirName());
  QDir filePath;
  if (!filePath.exists(filePathStr)) {
    filePath.mkpath(filePathStr);
  };
  
  QString newName = QString("%1/%2").arg(filePathStr).arg(finfo.fileName());
  
  QFileInfo targetInfo(newName);
  if (targetSize == T_ORIGINAL) {
    textureSet[fileName] = fileName;
    if (!targetInfo.exists()) {      
      if (!QFile::copy(fileName,newName) ) {
        RE_LOG_DEBUG() << "Warning: Reality could not collect the originally-sized texture " 
                       << QSS(fileName)  << " to " << QSS(newName);
        return(fileName);
      }
    }
  }
  else {
    // When downscaling we always use PNG for the output. It ensures that we don't loose
    // quality if the input file is in the jpeg format and it's smaller of TIFF or BMP.
    QString resizedName = QString("%1/%2.png").arg(filePathStr).arg(finfo.baseName());
    QImageWriter writer;
    writer.setFileName(resizedName);
    int newWidth = 512;
    switch (targetSize) {
      case T_256:
        newWidth = 256;
        break;
      case T_1K:
        newWidth = 1024;
        break;
      case T_2K:
        newWidth = 2048;
        break;
      default:
        break;
    };
    QImageReader reader(fileName);
    QSize imgSize = reader.size();       
    if (imgSize.width() > newWidth) {
      // Let's first check if the image is already there and already scaled, it will save us a lot of time to not 
      // redo what has been done before.
      bool needsResizing = true;
      QFileInfo newImageChecker(resizedName);
      if (newImageChecker.exists()) {
        QImageReader sizeChecker(resizedName);
        needsResizing = (sizeChecker.size().width() != newWidth);
      }
      if (needsResizing) {
        imgSize.scale(newWidth,newWidth,Qt::KeepAspectRatio);
        reader.setScaledSize(imgSize);
        writer.write(reader.read());
      }
      textureSet[fileName] = QDir(sceneDir).relativeFilePath(resizedName);
      return(textureSet[fileName]);        
    }
    else {
      if (!targetInfo.exists()) {
        if (!QFile::copy(fileName,newName) ) {
          RE_LOG_DEBUG() << "Warning: could not collect Reality texture " 
                         << QSS(fileName)  << " to " << QSS(newName);
          return(fileName);
        }
      }
    };
  }
  textureSet[fileName] = QDir(sceneDir).relativeFilePath(newName);
  return(textureSet[fileName]);
}

} // namespace
