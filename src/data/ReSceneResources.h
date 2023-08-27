/*
 *  ReTest.h
 *  ReGeomExport
 *
 *  Created by Paolo H Ciccone on 6/19/11.
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 *
 */

#ifndef RE_SCENE_RESOURCE_HELPER_H
#define RE_SCENE_RESOURCE_HELPER_H

#include <QtCore>
#include <QFileInfo>
#include <QHash>
#include "ReDefs.h"
#include "ReLogger.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * Class used to do the texture collection and to help set the location for the PLY files
 * generated during the binary export.
 */
class REALITY_LIB_EXPORT ReSceneResources  {

private:
  static ReSceneResources* instance;

  // ctor. Make it private so that that the object cannot be instantiated 
  // externally. This enforces the use of the <getInstance> method.
  ReSceneResources() {
    initialized = false;
  };

public:
  
  //! Method to retrieve the only instance of this class. This implements
  //! the Singleton pattern.
  static ReSceneResources* getInstance();

  ~ReSceneResources();
  
  QString getSceneName() const;
  void setSceneName(const QString sceneName);
  /**
   * Set the resource helper to work on a specific scene
   */
  void initialize();
  /**
   * Returns a boolean that signals if the resource helper has been initialized
   */
  bool isInitialized() { return(initialized); };
  /**
   * Reset the resource helper. Usually called after a render has been exporterd. It's useful
   * to avoid bugs caused by a previous initialization being out of sync with the current scene.
   */
  void reset() {
    initialized = false;
    sceneFileName = "";
  }

  QString collectTexture( const QString fileName, const ReTextureSize targetSize = T_ORIGINAL );
  QString getObjectsPath();
  QString getTexturesPath();
  QString getResourcePath();
  QString getResourceDirName();

  QString _newPath;
  inline QString getRelativePath( const QString& path ) {
    QDir d(sceneDir);
    _newPath = d.relativeFilePath(path);
    return _newPath;
  }

  void emptySLGKernelCache( const QString dirName );
  
  void printVersion() {
    RE_LOG_INFO() << "Scene Resource helper v.1.1";
  }
  
private:
  QString sceneFileName;
  QString sceneDir;
  QFileInfo sceneResourceInfo;
  QString objectsPath;
  QString texturesPath;
  QString resourceDirName;  // The relative name of the actual resource dir

  bool initialized;
  QHash<QString,QString> textureSet;
};

} // namespace

#endif
