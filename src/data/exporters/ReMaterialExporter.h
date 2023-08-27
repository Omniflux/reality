/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REMATERIALEXPORTER_H
#define REMATERIALEXPORTER_H

#include "ReDefs.h"
#include "ReMaterials.h"
#include "ReLogger.h"
#include <boost/any.hpp>
#include "reality_lib_export.h"

namespace Reality {

/**
 * Base class for all the material exporters. 
 * A material exporter is free to store its data in any format suitable.
 * Type erasure is used, via the \ref boost::any facility, to make this feature
 * possible. 
 * Type erasure is safe in this case because the material exporter is used
 * directly by the corresponding scene exporter, which knows what type to
 * expect. The type of data is obtained by using the appropriate version
 * of \ref boost::any_cast.
 *
 * By using this technique we can use a common interface and still support
 * a wide variety of exporters.
 */
class REALITY_LIB_EXPORT ReMaterialExporter {

public:
  // Constructor: ReMaterialExporter
  ReMaterialExporter() {
  };

  // Destructor: ReMaterialExporter
  virtual ~ReMaterialExporter() {
  };

  //! Export all the textures connected to this material. The idea is to 
  //! have a list of all the textures available before the list of materials.
  virtual void exportTextures( const ReMaterial* mat, 
                               boost::any& result, 
                               bool isForPreview = false ) = 0;
  
  //! Export the material using the native exporter's format
  virtual void exportMaterial( const ReMaterial* mat, boost::any& result ) = 0;
};

typedef QSharedPointer<ReMaterialExporter> ReMaterialExporterPtr;


} // namespace

#endif
