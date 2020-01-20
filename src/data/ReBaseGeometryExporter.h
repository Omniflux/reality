/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef BASE_GEOMETRY_EXPORTER_H
#define BASE_GEOMETRY_EXPORTER_H

#include <QtCore>
#include "ReDefs.h"
#include "ReMatrix.h"

namespace Reality {

/*
  Class: ReBaseGeometryExporter
 */

class ReGeometryBuffer;

class RE_LIB_ACCESS ReBaseGeometryExporter {

protected:
  // This is what we return for the export 
  QString materialData;

public:
  // Constructor: ReBaseGeometryExporter
  ReBaseGeometryExporter() {
  };
  // Destructor: ReBaseGeometryExporter
  virtual ~ReBaseGeometryExporter() {
  };

  //! Export a material. The returned string contains the material definition
  //! for the target renderer
  virtual QString& exportMaterial( const QString materialName, 
                                   const QString objectName,
                                   const QString shapeLabel,
                                   ReGeometryBuffer* geometryBuffer,
                                   const HostAppID scale ) = 0;
  /**
   * Add an instance of an object to the scene.
   * \param objectName The ID of the instance
   * \param transform. A map that lists the characteristicts of the 
   *                   transformation applied to the instance. The 
   *                   elements expected in the map are:
   *                   - xRot
   *                   - yRot
   *                   - zRot
   *                   - xScale
   *                   - yScale
   *                   - zScale
   *                   - xTran
   *                   - yTran
   *                   - zTran
   *                   - xOrder Sequence of the xRot in the transformation. One of 0,1,2
   *                   - yOrder Same as above
   *                   - zOrder Same as above
   */
  virtual QString& exportInstance( const QString& UNUSED(objectName), 
                                   const QVariantMap& UNUSED(transform),
                                   const HostAppID UNUSED(scale) )
  {
    return materialData;
  };

  //! Add an instance of an object to the scene.
  //! \param objectName The ID of the instance
  //! \param transform A transform matrix to apply to the instance
  virtual QString& exportInstance( const QString& UNUSED(objectName), 
                                   const ReMatrix& UNUSED(transform),
                                   const HostAppID UNUSED(scale) )
  {
    return materialData;
  };
};

} // namespace

#endif
