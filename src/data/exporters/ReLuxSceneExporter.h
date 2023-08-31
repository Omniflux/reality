/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef ReLuxSceneExporter_H
#define ReLuxSceneExporter_H

#include <boost/any.hpp>

#include "reality_lib_export.h"
#include "ReBaseSceneExporter.h"
#include "ReDefs.h"

namespace Reality {
  class ReMatrix;
}


namespace Reality {

/**
  This class exports a scene to LuxRender using the Lux scene file format
 */

class REALITY_LIB_EXPORT ReLuxSceneExporter : public ReBaseSceneExporter {

private:
  QString getRenderer();
  QString getCamera();
  QString getFilm( uint frameNo = -1 );
  QString getPixelFilter();
  QString getAccelerator( const LuxAccelerator accelerator );
  QString getSampler();
  QString getSurfaceIntegrator();
  QString getVolumeIntegrator();
  QString getLights();

public:
  // Constructor: ReLuxSceneExporter
  ReLuxSceneExporter( ReSceneData* scene ) : ReBaseSceneExporter(scene) {
  };
  // Destructor: ReLuxSceneExporter
  virtual ~ReLuxSceneExporter() {
  };

  void exportScene( const int frameNo, boost::any& sceneData );

  /**
   * Converts a transform matrix from the Poser/Studio format (right-sided)
   * to the LuxRender format (left-sided)
   *
   * This is done via matrix multiplication. If the third parameter is set to
   * true then the method applies an additional transform as suggested by
   * Jean-Philippe Grimaldi. The conversion is described as:
   *
   * Lux to Studio | Studio matrix | Studio to Lux
   * 1 0  0 0                        1  0 0 0
   * 0 0 -1 0              M         0  0 1 0
   * 0 1  0 0                        0 -1 0 0
   * 0 0  0 1                        0  0 0 1
   */
  static QString getMatrixString( const ReMatrix& matrix, 
                                  const HostAppID appID,
                                  const bool forInstance = false );

  void prepare() {

  }
  
  void cleanup() {

  };
};


}

#endif
