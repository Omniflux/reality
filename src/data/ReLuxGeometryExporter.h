/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_EXPORTER_H
#define LUX_EXPORTER_H

#include "ReBaseGeometryExporter.h"
#include "ReMatrix.h"

namespace Reality {

/*
  Class: ReLuxGeometryExporter
 */

class RE_LIB_ACCESS ReLuxGeometryExporter : public ReBaseGeometryExporter {

private:
  // Constructor: ReLuxGeometryExporter
  ReLuxGeometryExporter() {
  };

  static ReLuxGeometryExporter* instance;

  void writeLuxObject( ReGeometryBuffer* geometryBuffer, HostAppID scale, bool hasInvertedNormal );

  QString writePLYObject( QString objectName,
                          ReGeometryBuffer* geometryBuffer, 
                          HostAppID scale, 
                          bool hasInvertedNormals, 
                          bool writeBinary = true );

  void exportToLux( const QString materialName, 
                    const QString objectName,
                    const QString shapeName,                         
                    ReGeometryBuffer* geometryBuffer,
                    HostAppID scale );

public:
  // Destructor: ReLuxGeometryExporter
 ~ReLuxGeometryExporter() {
  };

  static ReLuxGeometryExporter* getInstance() {
    if (!instance) {
      instance = new ReLuxGeometryExporter();
    }

    return instance;
  }

  /**
   * When exporting an object the exporter must call
   * \ref exportObjectBegin() passing the uni ID of 
   * the object that needs to be exported. This operation is then
   * followed by a number of \ref exportMaterial() calls and
   * concluded with a call to \ref exportObjectEnd()
   * This sequence provides support for object instancing
   *
   * \param objectID The unique identifier of the object exporter
   * \return A string of bytes using the LuxRender syntax that is 
   *         used to export the object. This string needs to be
   *         written to a file.
   */
  QString& exportObjectBegin( const QString objectID );

  /**
   * This method closes the definition of an object and exports the object
   * to the renderer.
   *
   * \param objectID The unique identifier of the object exporter
   * \return A string of bytes using the LuxRender syntax that is 
   *         used to export the object. This string needs to be
   *         written to a file.
   */
  QString& exportObjectEnd( const QString objectID );

  /**
   * Method used to export a material. It returns the material definition
   * in the QString.
   *
   * \param materialName The name of the material to be exporter
   * \param objectName The name of the object that owns the material
   * \param shapeLabel The label used to refer to the shape exported. The 
   *                   shape is the set of polygons that form this material.
   * \param geometryBuffer The buffer containing the definition of polygons,
   *                       vertices, UV maps and normals. See \ref ReGeometryBuffer
   * \param scale The conversion scale identifier used to convert the geometry
   *              buffer unit to the exporter
   * \return A string of bytes that defines this material using the LuxRender
   *         syntax.
   */
  QString& exportMaterial( const QString materialName, 
                           const QString objectName,
                           const QString shapeLabel,
                           ReGeometryBuffer* geometryBuffer,
                           const HostAppID scale );
  
  //! Add an instance of an object to the scene.
  //! \param objectName The ID of the instance
  //! \param transform A transform matrix to apply to the instance
  QString& exportInstance( const QString& objectName, 
                           const QVariantMap& transform,
                           const HostAppID scale );

  QString& exportInstance( const QString& objectName, 
                           const ReMatrix& transform,
                           const HostAppID scale );
};


} // namespace

#endif
