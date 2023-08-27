/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef ReBaseSceneExporter_H
#define ReBaseSceneExporter_H

#include "ReDefs.h"
#include <QtCore>
#include <boost/any.hpp>
#include "reality_lib_export.h"

namespace Reality {

class ReSceneData;

/**
 * Base class for all exporters that generate a representation of the 
 * Reality scene. This is the interface used for both exporters that 
 * create versions of the scene for the renderers and for storing on disk.
 */
class REALITY_LIB_EXPORT ReBaseSceneExporter {
protected:
  ReSceneData* scene;

public:
  //! Constructor.
  //! The only function of this contructor is to store a reference
  //! to the \ref ReSceneData object
  ReBaseSceneExporter( ReSceneData* scene ) : scene(scene) {
    // Nothing
  };

  virtual ~ReBaseSceneExporter() {
  };

  virtual void exportScene( const int frameNo, boost::any& result ) = 0;

  //! This method is responsible for the setup work needed at the beginning
  //! of the export process
  virtual void prepare() = 0;
  
  //! This method is responsible for housekeeping at the end of the 
  //! export process.
  virtual void cleanup() = 0;
};

//! Defines a type of function that returns an instance of REBaseSceneExporter.
//! This special type of function is used to register additional types of
//! exporters that can be defined by special modules. For example, we can
//! have a Python exporter that generates a Python dict version of the scene
//! data. 
typedef ReBaseSceneExporter* (*ReSceneExporterCreator)(void);
//! Smart pointer
typedef QSharedPointer<ReBaseSceneExporter> ReBaseSceneExporterPtr;
}

#endif
