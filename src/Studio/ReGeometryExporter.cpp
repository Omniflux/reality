/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include <QHash>

#include "ReGeometryExporter.h"
#include "ReGeometryObject.h"
#include "ReSceneDataGlobal.h"
#include "ReGUID.h"
#include "ReLogger.h"
#include "ReDSTools.h"

#include "dzscene.h"
#include "dznode.h"
#include "dzobject.h"
#include "dzfigure.h"
#include "dzshape.h"
#include "dzfacetmesh.h"
#include "dzcustomdata.h"
#include "dzsettings.h"

using namespace Reality;
using namespace Reality::DS;

ReGeometryExporter::ReGeometryExporter( const ReDSMatCollection* flatMatList ) :
  flatMatList(flatMatList)
{
  auto dsNodes = dzScene->nodeListIterator();
  while( dsNodes.hasNext() ) {
    auto aNode = dsNodes.next();
    // Ignore cameras and lights, we are looking for geometry items
    if (aNode->inherits("DzCamera")) {
      continue;
    }
    auto dataItem = qobject_cast<DzSimpleElementData*>(
      aNode->findDataItem(RE_DS_NODE_GUID)
    );
    // There is no previously attached data then this is not a 
    // Reality-controlled node
    if (!dataItem) {
      continue;
    }
    auto settings = dataItem->getSettings();
    QString objID = settings->getStringValue(RE_DS_NODE_GUID);
    nodes[objID] = aNode;
  }
}

int ReGeometryExporter::getVertexIndex( const int vertexIndex, 
                                        const int normIndex,
                                        const int uvIndex ) 
{
  QString key = QString("%1%2").arg(vertexIndex).arg(uvIndex);
  if (vertMap.contains(key)) {
    return vertMap.value(key);
  }
  int idx = vertexList.count();
  vertexList.append(
    ReVertexInfoPtr( new ReVertexInfo(idx, vertexIndex, normIndex, uvIndex) )
  );
  vertMap[key] = idx;

  return idx;
};


void ReGeometryExporter::collectMaterialData( const int matIndex ) {
  startMaterial();

  auto indices = materialGroups.value(matIndex);
  int numPolys = indices.count();

  DzFacet* faces = geom->getFacetsPtr();

  // Studio reduces every mesh to quads if it contains n-gons
  int newPoly[4];
  int triSize = sizeof(ReTriangle);
  for (int i = 0; i < numPolys; i++) {
    auto poly = faces[indices[i]];
    int faceVerts;
    // The polygons is a triangle if the fourth vertex id is -1
    bool isQuad = poly.m_vertIdx[3] > -1;
    faceVerts = (isQuad ? 4 : 3);
    for (int l = 0; l < faceVerts; l++) {
      newPoly[l] = getVertexIndex(
        poly.m_vertIdx[l], poly.m_normIdx[l], poly.m_uvwIdx[l]
      );
    }
    ReTrianglePtr tri = ReTrianglePtr( new ReTriangle() );
    // 0, 1, 2
    memcpy(tri->a, newPoly, triSize);
    triList.append(tri);
    // Triangulate the quads
    if (isQuad) {
      // 0, 2, 3
      tri = ReTrianglePtr( new ReTriangle() );
      tri->a[0] = newPoly[0];
      tri->a[1] = newPoly[2];
      tri->a[2] = newPoly[3];
      triList.append(tri);
    }
  }
}

bool ReGeometryExporter::setup( const DzNode* node ) {
  dzObj = node->getObject();
  if (!dzObj) {
    RE_LOG_WARN() << "Could not find object for the node " 
                  << node->getName().toStdString();
    return false;
  }
  shape = dzObj->getCurrentShape();
  if (!shape) {
    RE_LOG_WARN() << "Could not find shape for node "
                  << node->getName().toStdString();
    return false;
  }
  geom = static_cast<DzFacetMesh*>(dzObj->getCachedGeom());
  if (!geom) {
    RE_LOG_WARN() << "Could not find geometry data for node "
                  << node->getName().toStdString();
    return false;
  }
  return true;
}

void ReGeometryExporter::getMaterialGroups( const DzNode* node ) {
  materialGroups.clear();

  int numFacets = geom->getNumFacets();
  auto facets   = geom->getFacetsPtr();
  auto flags    = geom->getFacetFlagsPtr();

  for (int i = 0; i < numFacets; i++) {
    if (flags[i] & DZ_HIDDEN_FACE_BIT) {
      continue;
    }
    auto mat = shape->getAssemblyMaterial(facets[i].m_materialIdx);
    materialGroups[mat->getIndex()].append(i);
    // materialNameTable[facets[i].m_materialIdx] = mat->getName();
  }
} 

void ReGeometryExporter::exportObject( const QString& objName, 
                                       const ReGeometryObjectPtr obj ) 
{
  auto node = nodes.value(objName);

  if (!node) {
    return;
  }

  // Skip invisible objects
  if (!obj->isVisible()) {
    return;
  }
  // Skip figures that are geo-grafted
  DzFigure* figure = qobject_cast<DzFigure*>(node);
  if ( figure && figure->isGraftFollowing() ) {
    auto followTarget = figure->getFollowTarget();
    if ( followTarget && 
         followTarget->isVisible() && 
         followTarget->isVisibileInRender() ) 
    {
      return;
    }
  } 

  QString GUID = ReGUID::getGUID(node);

  // Double-check that we have indeed the right node
  if (GUID == obj->getInternalName()) {
    if ( !setup(node) ) {
      return;
    }
    getMaterialGroups(node);

    RealitySceneData->renderSceneObjectBegin(GUID);

    // Iterate through all the materials
    ReMaterialGroupIterator i(materialGroups);
    while( i.hasNext() ) {
      i.next();
      int matIndex = i.key();

      ReMaterialPtr reMat = flatMatList->getMaterial(matIndex);
      if ( !(!reMat.isNull() && reMat->isVisibleInRender()) ) {
        continue;
      }
      collectMaterialData( matIndex );
      exportMaterial(reMat->getName(), reMat->getParent()->getInternalName());
    }
    RealitySceneData->renderSceneObjectEnd(GUID);
  }
};


void ReGeometryExporter::exportMaterial( const QString& matName, 
                                         const QString objName ) 
{
  RealitySceneData->newGeometryBuffer( matName.toUtf8(), 
                                       vertMap.count(), 
                                       triList.count(),
                                       true );
  float* verts   = RealitySceneData->getGeometryVertexBuffer();
  float* normals = RealitySceneData->getGeometryNormalBuffer();
  ReUVPoint* uvs = RealitySceneData->getGeometryUVPointBuffer();
  int* tris      = RealitySceneData->getGeometryFaceBuffer();
  DzMap* dzUVs   = geom->getUVs();

  // Copy the triangles
  int triCount = triList.count();
  int triSize = sizeof(ReTriangle);
  for (int i = 0; i < triCount; i++) {
    auto tri = triList[i];
    memcpy(tris+(i*3), tri->a, triSize);
  }
  // Copy the vertex data
  int vertCount = vertexList.count();
  for (int i = 0; i < vertCount; i++) {
    auto vertInfo = vertexList[i];
    // DzPnt3* vx = dzVerts+vertInfo->oldVertexIndex;
    auto v = geom->getVertex(vertInfo->oldVertexIndex);
    verts[i*3]   = v.m_x;
    verts[i*3+1] = v.m_y;
    verts[i*3+2] = v.m_z;

    // v = dzNorms+vertInfo->oldVertexIndex;
    v = geom->getNormal(vertInfo->normIndex);
    normals[i*3]   = v.m_x;
    normals[i*3+1] = v.m_y;
    normals[i*3+2] = v.m_z;

    auto uvPoint = dzUVs->getPnt2Value(vertInfo->uvIndex);
    uvs[i][0] = uvPoint[0];
    uvs[i][1] = uvPoint[1];
  }
  RealitySceneData->renderSceneExportMaterial( 
    matName.toUtf8(),
    objName.toUtf8(), 
    QString("%1::%2").arg(objName).arg(matName).toUtf8(),
    Reality::DAZStudio 
  );
  // Cleanup
  startMaterial();
}



























