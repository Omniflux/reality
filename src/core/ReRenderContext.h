/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_RENDER_CONTEXT_H
#define RE_RENDER_CONTEXT_H

#include <QStringList>

#include "ReDefs.h"


/**
 * This class provides information about the render process. It's meant to 
 * collect and provide information that can be useful to optimize the rendering
 * or to simply keep relevant data current while rendering an animation.
 *
 * The life-span of this object is limited to the rendering performed. If Reality
 * is rendering a frame then the context will be cleared once the frame is 
 * exported, if the render is an animation then the context will be deleted once
 * the last frame of the animation has been exported.
 */
class ReRenderContext {

private:
  //! True if the render uses object instances.
  bool hasInstancesFlag;

  //! True if the render uses sunlight
  bool hasSunFlag;

  //! True if the render uses IBL
  bool hasIBLFlag;

  RE_UINT numMeshLights;
  RE_UINT numSpotLights;
  RE_UINT numInfiniteLights;

  //! True if the render uses OpenCL
  bool usesOCLFlag;

  //! True if the render uses LuxCore
  bool usesLuxCoreFlag;

  //! List of object IDs that refer to objects for which multiple instances
  //! are present in the scene. This list is used to determine that we need 
  //! to include the special declarations use to subsequently instantiate the
  //! object.
  QStringList objectInstatiators;

  //! Constructor: ReRenderContext
  ReRenderContext() {
    init();
  };

  //! Singleton implementation
  static ReRenderContext* instance;

  QString sceneFileName;
  QString imageFileName;

protected:

public:

  //! Scans the scene and finds out if there are instances. If there are, they
  //! are collected so that the exporter will have a way of finding out which
  //! object is the source for instances and when it's the case to export an
  //! instance and when it's just simpler to export an anonymous object.
  void init();

  //! Utility method to check if there are instances in this render
  inline bool hasInstances() {
    return hasInstancesFlag;
  }

  //! Returns if the given object creates instances
  inline bool isInstantiator( const QString& objID ) {
    return objectInstatiators.contains(objID);
  }

  inline bool hasSun() {
    return hasSunFlag;
  }

  inline bool hasIBL() {
    return hasIBLFlag;
  }

  inline bool usesOpenCL() {
    return usesOCLFlag;
  }

  inline bool usesLuxCore() {
    return usesLuxCoreFlag;
  }

  inline QString getImageFileName() {
    return imageFileName;
  }

  inline QString getSceneFileName() {
    return sceneFileName;
  }

  RE_UINT getMeshLightCount() {
    return numMeshLights;
  }

  RE_UINT getSpotLightCount() {
    return numSpotLights;
  }

  RE_UINT getInfiniteLightCount() {
    return numInfiniteLights;
  }

  //! Access to the singleton instance
  static ReRenderContext* getInstance();

};


#endif
