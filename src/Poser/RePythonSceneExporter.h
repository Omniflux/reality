/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#ifndef RE_PYTHON_SCENE_EXPORTER_H
#define RE_PYTHON_SCENE_EXPORTER_H

#include "exporters/ReBaseSceneExporter.h"
#include <boost/any.hpp>
  
namespace Reality {

/**
  Class: RePythonSceneExporter
 */

class RePythonSceneExporter : public ReBaseSceneExporter {

public:
  //! Constructor: RePythonSceneExporter
  RePythonSceneExporter() : ReBaseSceneExporter(NULL) {
  };

  //! Destructor: RePythonSceneExporter
 ~RePythonSceneExporter() {
  };

  void exportScene( const int frameNo, boost::any& sceneData );

  void prepare() {

  }
  
  void cleanup() {

  };
  
};

} // namespace

#endif
