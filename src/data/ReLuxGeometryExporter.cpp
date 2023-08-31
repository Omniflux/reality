/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLuxGeometryExporter.h"

// For isnan/_isnan support
#if defined(_WIN32)
#include <float.h>
#else
#include <cmath>
#endif

#include <QStringBuilder>

#include "ply/rply.h"
#include "ReDisplaceableMaterial.h"
#include "ReLightMaterial.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReSceneResources.h"
#include "ReTools.h"
#include "exporters/ReLuxSceneExporter.h"
#include "exporters/lux/ReLightExporter.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "exporters/lux/ReLuxMaterialExporterFactory.h"
#include "exporters/lux/ReLuxTextureExporter.h"


// NAN checking
#if defined(_WIN32)
#define CHECK_NAN(x) ( _isnan(x) ? 0.0 : x )
#define IS_NAN _isnan
#else
#define CHECK_NAN(x) ( isnan(x) ? 0.0 : x )
#define IS_NAN isnan
#endif

namespace Reality {

/*
void generateNormal(ReGeometryBuffer* geometryBuffer, int triangle) {
  // get difference vectors:
  ReVectorF v1,v2,v3;

  v1[0] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][0] ][0];
  v1[1] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][0] ][1];
  v1[2] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][0] ][2];

  v2[0] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][1] ][0];
  v2[1] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][1] ][1];
  v2[2] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][1] ][2];

  v3[0] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][2] ][0];
  v3[1] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][2] ][1];
  v3[2] = geometryBuffer->vertices[ geometryBuffer->triangles[triangle][2] ][2];

  // Difference vector
  float dx1 = v2[0] - v1[0]; float dy1 = v2[1] - v1[1]; float dz1 = v2[2] - v1[2];
  float dx2 = v3[0] - v1[0]; float dy2 = v3[1] - v1[1]; float dz2 = v3[2] - v1[2];

  //cross-product:
  float ox = dy1 * dz2 - dz1 * dy2;
  float oy = dz1 * dx2 - dx1 * dz2;
  float oz = dx1 * dy2 - dy1 * dx2;

  //normalize:
  float mag = sqrt( ox * ox + oy * oy + oz * oz );
  ox /= mag;
  oy /= mag;
  oz /= mag;

  geometryBuffer->normals[ geometryBuffer->triangles[triangle][0] ][0] = ox;
  geometryBuffer->normals[ geometryBuffer->triangles[triangle][0] ][1] = oy;
  geometryBuffer->normals[ geometryBuffer->triangles[triangle][0] ][2] = oz;

  geometryBuffer->normals[ geometryBuffer->triangles[triangle][1] ][0] = ox;
  geometryBuffer->normals[ geometryBuffer->triangles[triangle][1] ][1] = oy;
  geometryBuffer->normals[ geometryBuffer->triangles[triangle][1] ][2] = oz;

  geometryBuffer->normals[ geometryBuffer->triangles[triangle][2] ][0] = ox;
  geometryBuffer->normals[ geometryBuffer->triangles[triangle][2] ][1] = oy;
  geometryBuffer->normals[ geometryBuffer->triangles[triangle][2] ][2] = oz;

}
*/

ReLuxGeometryExporter* ReLuxGeometryExporter::instance = NULL;

void ReLuxGeometryExporter::writeLuxObject( ReGeometryBuffer* geometryBuffer, 
                                            HostAppID scale,
                                            bool hasInvertedNormals ) {
  // The number of vertices exported must be equal to the number of UV points.
  // So, if a there are no enough vertices we then repeat the vertex that
  // are associated with a give UV point. To do so we create a buffer to store
  // the extra vertices. This buffer will be then appended at the end of 
  // the list of vertices exported. 
  // The way we build this buffer is as follows:
  //   - We scan the list of triangles to export. For each vertex in the triangle
  //     we look up the same vertex in the UV Map triangle. 
  //   - If the index of that UV point is higher than the last vertex it means
  //     that this is a vertex that we need to duplicate
  //   - We grab the vertex and then add it to the <extraVerts> buffer. This
  //     buffer has only enough elements to include the difference between
  //     the number of UV points and the number of geometry vertices
  // 
  //  For example, the typical cube has 8 vertices, 12 triangles and 14 UV points
  //  From UV point 0 to 7 we have a match with the corresponding geometry vertices
  //  Since there is a difference of 6 elements between the UV points and the vertices
  //  we allocate an <extraVerts> buffer of 6 elements. 
  //  When we find a UV triangle that references, for example, point #12 we know that
  //  we need to find the corresponding geometry vertex to duplicate. 
  //  We look up in the geometry triangle and locate the same point in it. That point
  //  gives us the index of the vertices. Let's say that it is index #6. We grab the
  //  XYZ coordinates of that vertices and we copy it into the <extraVerts> buffer.
  //  To find at what index in the buffer we need to copy the vertex we use the 
  //  following formula:
  //     vertex_index = uv_point_index - number_of_vertices
  //  So, UV point 12 - 8 = 4, the fifth element in the buffer, and that's were
  //  it needs to be copied.
  //  When we output the array of vertices we use the standard <geometry->vertices>
  //  buffer first and then add the <extraVerts> at the end. This completes the
  //  export with the right amount of vertices and UV points for Lux.

  materialData += "\"integer triindices\" [\n";

  // Scan through the list of triangles...
  for (int i = 0; i < geometryBuffer->numTriangles; ++i) {
    materialData += QString("%1 %2 %3\n")
                      .arg(geometryBuffer->triangles[i].a[0])
                      .arg(geometryBuffer->triangles[i].a[1])
                      .arg(geometryBuffer->triangles[i].a[2]);

    // generateNormal(geometryBuffer, i);
  }
  materialData += "]\n";

  QString uv,p,n;
  // Write the UV point array
  bool hasUVs = geometryBuffer->uvmap != NULL;
  uv = "\"float uv\" [\n";
  p  = "\"point P\" [\n";
  n  = "\"normal N\" [\n";
  for (int i = 0; i < geometryBuffer->numVertices; ++i) {
    if ( hasUVs ) {
      uv += QString("%1 %2\n")
              .arg(CHECK_NAN(geometryBuffer->uvmap[i][0]),10,'f',8)
              .arg(CHECK_NAN(geometryBuffer->uvmap[i][1]),10,'f',8);
    }

    p += QString("%1 %2 %3\n")
           .arg(convertUnit(geometryBuffer->vertices[i][0], scale),8,'f',6)
           .arg(-convertUnit(geometryBuffer->vertices[i][2], scale),8,'f',6)
           .arg(convertUnit(geometryBuffer->vertices[i][1], scale),8,'f',6);  
    if (hasInvertedNormals) {
      n += QString("%1 %2 %3\n")
             .arg(IS_NAN(geometryBuffer->normals[i][0]) ? 0 : -geometryBuffer->normals[i][0],
                  8,'f',6)
             .arg(IS_NAN(geometryBuffer->normals[i][2]) ? 0 : geometryBuffer->normals[i][2],
                  8,'f',6)
             .arg(IS_NAN(geometryBuffer->normals[i][1]) ? 0 : -geometryBuffer->normals[i][1],
                  8,'f',6);  
    }
    else {
      n += QString("%1 %2 %3\n")
             .arg(IS_NAN(geometryBuffer->normals[i][0]) ? 0 : geometryBuffer->normals[i][0],
                  8,'f',6)
             .arg(IS_NAN(geometryBuffer->normals[i][2]) ? 0 : -geometryBuffer->normals[i][2],
                  8,'f',6)
             .arg(IS_NAN(geometryBuffer->normals[i][1]) ? 0 : geometryBuffer->normals[i][1],
                  8,'f',6);      
    }
  }
  uv += "]\n";
  p += "]\n";
  n += "]\n";

  // Write the geometry
  if (hasUVs) {
    materialData += p % n % uv;    
  }
  else {
    materialData += p % n;
  }  
}

QString ReLuxGeometryExporter::writePLYObject( QString objectName,
                                               ReGeometryBuffer* geometryBuffer, 
                                               HostAppID scale, 
                                               bool hasInvertedNormals, 
                                               bool writeBinary ) 
{
  ReSceneResources* srh = ReSceneResources::getInstance();
  // Replaces the ":" that can be in some filenames and that can cause issues 
  // with Windows
  QString plyFileName = QString("%1/%2.ply")
                        .arg(srh->getObjectsPath())
                        .arg(sanitizeFileName(objectName));

  p_ply plyFile = ply_create(plyFileName.toUtf8(), (writeBinary ? PLY_LITTLE_ENDIAN : PLY_ASCII), NULL );
  ply_add_comment(plyFile, "File created by Reality plug-in");
  // Write the header
  ply_add_element(plyFile, "vertex", geometryBuffer->numVertices);
  ply_add_scalar_property(plyFile, "x",  PLY_FLOAT);
  ply_add_scalar_property(plyFile, "y",  PLY_FLOAT);
  ply_add_scalar_property(plyFile, "z",  PLY_FLOAT);

  // Properties of the normals
  ply_add_scalar_property(plyFile, "nx", PLY_FLOAT);
  ply_add_scalar_property(plyFile, "ny", PLY_FLOAT);
  ply_add_scalar_property(plyFile, "nz", PLY_FLOAT);      

  bool hasUVs = geometryBuffer->uvmap != NULL;

  if ( hasUVs ) {
    ply_add_scalar_property(plyFile, "s",  PLY_FLOAT);  // That's how Lux is marking the UV entries
    ply_add_scalar_property(plyFile, "t",  PLY_FLOAT);
  }
  ply_add_element(plyFile, "face", geometryBuffer->numTriangles);
  ply_add_list_property(plyFile, "vertex_indices", PLY_UCHAR, PLY_UINT);
  ply_write_header(plyFile);

  // Write the vertices
  for (int i = 0; i < geometryBuffer->numVertices; ++i) {
    ply_write(plyFile, convertUnit(geometryBuffer->vertices[i][0], scale));
    ply_write(plyFile, -convertUnit(geometryBuffer->vertices[i][2], scale));
    ply_write(plyFile, convertUnit(geometryBuffer->vertices[i][1], scale));
    if (hasInvertedNormals) {
      ply_write(plyFile, IS_NAN(geometryBuffer->normals[i][0]) ? 0 : -geometryBuffer->normals[i][0]);
      ply_write(plyFile, IS_NAN(geometryBuffer->normals[i][2]) ? 0 : geometryBuffer->normals[i][2]);
      ply_write(plyFile, IS_NAN(geometryBuffer->normals[i][1]) ? 0 : -geometryBuffer->normals[i][1]);
    }
    else {
      ply_write(plyFile, IS_NAN(geometryBuffer->normals[i][0]) ? 0 : geometryBuffer->normals[i][0]);
      ply_write(plyFile, IS_NAN(geometryBuffer->normals[i][2]) ? 0 : -geometryBuffer->normals[i][2]);
      ply_write(plyFile, IS_NAN(geometryBuffer->normals[i][1]) ? 0 : geometryBuffer->normals[i][1]);
    }
    if (hasUVs) {
      ply_write(plyFile, CHECK_NAN(geometryBuffer->uvmap[i][0]));
      ply_write(plyFile, CHECK_NAN(geometryBuffer->uvmap[i][1]));
    }
  }

  // Output the tris
  for (int i = 0; i < geometryBuffer->numTriangles; ++i) {
    ply_write(plyFile,3);
    ply_write(plyFile,geometryBuffer->triangles[i].a[0]);
    ply_write(plyFile,geometryBuffer->triangles[i].a[1]);
    ply_write(plyFile,geometryBuffer->triangles[i].a[2]);
  };

  ply_close(plyFile);
  return(plyFileName);
}

void ReLuxGeometryExporter::exportToLux( const QString materialName, 
                                         const QString objectName,
                                         const QString shapeName,
                                         ReGeometryBuffer* geometryBuffer,
                                         HostAppID scale ) {
  auto sceneResources = ReSceneResources::getInstance();
  materialData.clear();
  if (geometryBuffer->numVertices == 0) {
    geometryBuffer->reset();
    return;
  }

  ReGeometryObjectPtr obj = RealitySceneData->getObject(objectName);
  ReMaterialPtr mat = obj->getMaterial(materialName);
  
  // 
  if (mat.isNull()) {
    materialData += QString("#! Could not find data for material %1:%2\n").arg(objectName).arg(materialName);
    geometryBuffer->reset();
    return;
  }

  materialData = QString("# Mat %1 (%2). %3 polys\n")
                   .arg(materialName)
                   .arg(mat->getTypeAsString())
                   .arg(geometryBuffer->numTriangles);

  materialData += "AttributeBegin\n";
  QString innerVol = mat->getInnerVolume();
  QString outerVol = mat->getOuterVolume();
  if (innerVol != "") {
    materialData += QString("Interior \"%1\"\n").arg(innerVol);
  }
  if (outerVol != "") {
    materialData += QString("Exterior \"%1\"\n").arg(outerVol);
  }

  // Support for meshlight's inverted normals
  bool hasInvertedNormals = false;
  // Mesh Light
  ReLightMaterialPtr matLight = obj->getLight(materialName);
  if (!matLight.isNull()) {
    ReLightPtr meshLight = matLight->getLight();
    if (meshLight->isLightOn()) {
      ReLuxLightExporter* lightExporter = ReLuxLightExporter::getInstance();
      hasInvertedNormals = meshLight->getInvertedNormals();
      materialData += lightExporter->exportLight(
        meshLight
      );

      if (!RealitySceneData->isOCLRenderingON() && meshLight->hasAlphaChannel()) {
        // We have a global Null material at the beginning of the scene that is used ti avoid
        // creating null materials for each mesh light. The material for each light is simply
        // that null, which has the effect of creating an alpha channel for each pixel of the
        // associated texture, if there is one, that is black.
        materialData += "NamedMaterial \"RealityNull\"\n";
      }
    }
  }
  else {
    materialData += QString("NamedMaterial \"%1\"\n").arg(mat->getUniqueName());
  }
  // Test to see if the material has modifiers
  ReModifiedMaterialPtr dmat = mat.dynamicCast<ReModifiedMaterial>();
  // Check if the material has the Light Emission flag on. In that case
  // configure the material to be an emitter
  if (!dmat.isNull() && dmat->isEmittingLight()) {
    materialData += QString("LightGroup \"%1:%2\"\n")
                      .arg(obj->getName())
                      .arg(materialName);
    materialData += QString(
                      "AreaLightSource \"area\" \"float gain\" [%1] "
                      "\"float efficacy\" [17] \"float power\" [100] "
                    )
                    .arg(dmat->getLightGain());
    ReTexturePtr ambTex = ReLuxTextureExporter::getTextureFromCache(
                            dmat->getAmbientMap()
                          );
    if (!ambTex.isNull()) {
      materialData += QString("\"texture L\" [\"%1\"]")
                        .arg(ambTex->getUniqueName());
    }
    materialData += "\n";
  }  

  GeometryFileFormat gFileFormat = RealitySceneData->getGeometryFormat();

  QString meshType = "mesh"; // Standard for LuxNative
  if ( gFileFormat == BinaryPLY || gFileFormat == TextPLY ) {
    meshType = "plymesh";
  }  
  materialData += QString("Shape \"%1\" \"string name\" [\"%2\"]\n").arg(meshType).arg(objectName);

  // Subdivision
  auto matExporter = ReLuxMaterialExporterFactory::getExporter(mat.data());
  materialData += matExporter->getSubdivision(mat.data());
  // Displacement
  materialData += matExporter->getDisplacementClause(mat.data());

  if (gFileFormat == LuxNative) {
    writeLuxObject(geometryBuffer, scale, hasInvertedNormals);    
  }
  else if ( gFileFormat == BinaryPLY || gFileFormat == TextPLY ) {
    materialData += QString("\"string filename\" [\"%1\"]\n")
                      .arg(sceneResources->getRelativePath(
                            writePLYObject( 
                              QString("%1-%2").arg(objectName).arg(materialName), 
                              geometryBuffer, 
                              scale, 
                              hasInvertedNormals, 
                              gFileFormat == BinaryPLY 
                            )
                          ));
  }

  materialData += "AttributeEnd\n";

  geometryBuffer->reset();
}


QString& ReLuxGeometryExporter::exportMaterial( const QString materialName, 
                                                const QString objectName,
                                                const QString shapeName,
                                                ReGeometryBuffer* geometryBuffer,
                                                const HostAppID scale ) 
{
  exportToLux(materialName, objectName, shapeName, geometryBuffer, scale);
  return materialData;
}

QString& ReLuxGeometryExporter::exportObjectBegin( const QString objectID ) {
  materialData.clear();
  materialData += QString("ObjectBegin \"%1\"\n").arg(objectID);
  return materialData;
};

QString& ReLuxGeometryExporter::exportObjectEnd( const QString objectID ) {
  materialData.clear();
  materialData += QString("ObjectEnd\nObjectInstance \"%1\"\n").arg(objectID);
  materialData.squeeze();
  return materialData;
};


QString& ReLuxGeometryExporter::exportInstance( const QString& objectName, 
                                                   const ReMatrix& trans,
                                                   const HostAppID appID ) 
{
  materialData.clear();
  auto obj = RealitySceneData->getObject(objectName);
  if (obj.isNull()) {
    return materialData;
  }
  if (!obj->isInstance()) {
    return materialData;
  }
  materialData += "TransformBegin\n";
  materialData += ReLuxSceneExporter::getMatrixString(trans, appID, true);
  materialData += QString("ObjectInstance \"%1\"\n").arg(obj->getInstanceSourceID());
  materialData += "TransformEnd\n";
  return materialData;
}

/**
 * Add an instance of an object to the scene.
 * \param objectName The ID of the instance
 * \param transform. A map that lists the characteristics of the
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
QString& ReLuxGeometryExporter::exportInstance( const QString& objectName, 
                                                   const QVariantMap& trans,
                                                   const HostAppID appID ) 
{
  materialData.clear();
  auto obj = RealitySceneData->getObject(objectName);
  if (obj.isNull()) {
    return materialData;
  }
  if (!obj->isInstance()) {
    return materialData;
  }
  materialData += "TransformBegin\n";
  materialData += QString("Translate %1 %2 %3\n")
                    .arg(convertUnit(trans["xTran"].toFloat(), appID))
                    .arg(-convertUnit(trans["zTran"].toFloat(), appID))
                    .arg(convertUnit(trans["yTran"].toFloat(), appID));

  const short int numRots = 3;
  QString rots[numRots];
  float rotVal[numRots];
  rotVal[0] =  trans["xRot"].toFloat(),
  rotVal[1] =  trans["yRot"].toFloat(),
  rotVal[2] = -trans["zRot"].toFloat();

  rots[trans["xOrder"].toInt()] = QString("Rotate %1 1 0 0\n")
                                    .arg(rotVal[0]);
  rots[trans["yOrder"].toInt()] = QString("Rotate %1 0 0 1\n")
                                    .arg(rotVal[1]);
  rots[trans["zOrder"].toInt()] = QString("Rotate %1 0 1 0\n")
                                    .arg(rotVal[2]);
  for (int i = 0; i < numRots; i++) {
    if (rotVal[i]) {
      materialData += rots[i];
    }
  }
  // Scaling must be the last operation
  float scale = trans["Scale"].toFloat();
  materialData += QString("Scale %1 %2 %3\n")
                    .arg(trans["xScale"].toFloat() * scale)
                    .arg(trans["zScale"].toFloat() * scale)
                    .arg(trans["yScale"].toFloat() * scale);

  materialData += QString("ObjectInstance \"%1\"\n").arg(obj->getInstanceSourceID());
  materialData += "TransformEnd\n";
  return materialData;
}


} // namespace
