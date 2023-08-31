/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_GEOMETRY_EXPORTER_H
#define RE_GEOMETRY_EXPORTER_H

#include <QHash>
#include <QSharedPointer>

#include "ReGeometry.h"

class DzFacetMesh;
class DzNode;
class DzObject;
class DzShape;

namespace Reality {
  class ReDSMatCollection;
  class ReGeometryObject;
  typedef QSharedPointer<ReGeometryObject> ReGeometryObjectPtr;
}


namespace Reality {

/**
 * This struct is used to store the information about a vertex during the 
 * reordering of the vertices for a given material. LuxRender requires the
 * vertices to be coupled with their UV. In fact the UV points drive the
 * scene description. This means that we need to provide unique pairs of
 * <uv point, vertex>, which in turn changes the order of the vertices 
 * from their original arrangement. 
 */
struct ReVertexInfo {
  int oldVertexIndex;
  int newVertexIndex;
  int normIndex;
  int uvIndex;

  ReVertexInfo( int newVertexIndex, int oldVertexIndex, int oldNormIndex, int uvIndex ) 
    : oldVertexIndex(oldVertexIndex)
    , newVertexIndex(newVertexIndex)
    , normIndex(oldNormIndex)
    , uvIndex(uvIndex)
  {
    // nothing    
  }
};
//! Smart pointer to a vertex info.
typedef QSharedPointer<ReVertexInfo> ReVertexInfoPtr;

//! List of vertex info
typedef QList<ReVertexInfoPtr> ReVertexList;

//! Map used to reorder the vertex indices
typedef QHash<QString, int> ReVertexMap;  

//! List of facets belonging to a single material. The key is the material
//! index used by Studio. The value is the facet index referred to the current geometry

// typedef QHash< QString, QList<int> > ReMaterialGroupTable;
typedef QHash< int, QList<int> > ReMaterialGroupTable;
typedef QHashIterator< int, QList<int> > ReMaterialGroupIterator;

/**
 * This class implements the geometry exporter of Reality for Studio
 */
class ReGeometryExporter {

public:
  typedef QHash<QString, DzNode*> NodeDict;
  typedef QHashIterator<QString, DzNode*> NodeDictIterator;

private:
  DzObject* dzObj;
  DzShape* shape;
  DzFacetMesh* geom;

  const ReDSMatCollection* flatMatList;

  //! Initializes the dzObj and shape vars
  bool setup( const DzNode* node );

  /**
   * Collects the polygons belonging to a group and converts them to 
   * triangles. The triangles are stored in the \ref triList container
   * as instances of ReTrianglePtr. This method also re-orders the 
   * vertices based on the LuxRender logic.
   */
  void collectMaterialData( const int matIndex );

  ReVertexMap vertMap;
  ReTriangleList triList;
  ReVertexList vertexList;

  //! List of facets associated with each material
  ReMaterialGroupTable materialGroups;

  //! A dictionary of DzNode objects keyed by the Reality internal 
  //! object name. This is used to iterate through the list of 
  //! objects to export. The contructor initializes this dictionary.
  NodeDict nodes;

public:
  //! Constructor: ReGeometryExporter
  ReGeometryExporter( const ReDSMatCollection* flatMatList );

  //! Destructor: ReGeometryExporter
 ~ReGeometryExporter() {
  };

  NodeDict getNodes() {
    return nodes;
  }

  void exportObject( const QString& objName, const ReGeometryObjectPtr obj );
  void exportMaterial( const QString& matName, const QString objName  );

  inline void startMaterial() {
    // newVertexIndex = 0;
    vertMap.clear();
    vertexList.clear();
    triList.clear();
  }

  int getVertexIndex( const int vertexIndex, 
                      const int normIndex,
                      const int uvIndex );
  
  //! Creates a list of polygons grouped by material. The list is actually
  //! including the index of the polygons inside the set pointed by
  //! DzFacetMesh::getFacetPtr()
  void getMaterialGroups( const DzNode* node );
};

} // namespace

#endif
