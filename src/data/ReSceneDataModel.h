/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_SCENEDATAMODEL_H
#define RE_SCENEDATAMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include "ReGeometryObject.h"
#include "ReIPCCommands.h"
#include "RealityPanel/RealityDataRelay.h"


#define RE_MB_COL_MATERIAL 0
#define RE_MB_COL_TYPE     1
#define RE_MB_COL_EDITED   2
#define RE_MB_COL_HIDDEN   3
#define RE_MB_COL_EMITTER  4
#define RE_MB_COL_ACSEL    5
#define RE_MB_NUM_COLUMS   6


namespace Reality {

/**
 This is the Tree used as the foundation of the data model
 Each node can either point to an Object, which is a "parent"
 node, or a material, which is a "child node".

 The nodes have a void pointer that points to the original 
 element in the data model. This structure makes it easy
 to reorganize the data in a way that is friendly to the
 QTreeView widget.

 */
class TreeItem {

public:
  QList<TreeItem*> children;

  enum NodeType {
    NodeIsObject,
    NodeIsMaterial
  };

  NodeType nodeType;
  void* pointer;
  TreeItem* parent;

  TreeItem( NodeType nType, void* dataPointer, TreeItem* parent = 0 ) :
    nodeType(nType), 
    pointer(dataPointer), 
    parent(parent) 
  {
    // Nothing
  }

  ~TreeItem() {
    qDeleteAll(children);
    children.clear();
  }

  void addChild( TreeItem* child ) {
    children.append(child);
  }

  void removeChild( const quint16 i ) {
    delete children.takeAt(i);
  }

  inline TreeItem* getChild(const qint16 num) {
    return children[num];
  }

  inline qint16 getNumChildren() {
    return children.count();
  }

  inline TreeItem* getParent() {
    return parent;
  }
  
  quint16 getRow() const {
    if (parent) {
      if (parent->children.count() == 0) {
        return 0;
      }
      return parent->children.indexOf(const_cast<TreeItem*>(this));
    }
    return 0;
  }

  // Method: removeMaterial
  //   Removes a node that corresponds to a give material. Used when
  //   a material is changed in the host app while Reality is running
  void removeMaterial( const QString& objectID, const QString& materialName ) {
    QListIterator<TreeItem*> i(children);
    while(i.hasNext()) {
      TreeItem* node = i.next();
      ReGeometryObject* obj = static_cast<ReGeometryObject*>(node->pointer);
      if (obj->getInternalName() == objectID ) {
        // Found the right parent node, remove the node that links to the material
        int matCount = node->getNumChildren();
        for (int i = 0; i < matCount; ++i) {
          TreeItem* matNode = node->getChild(i);
          if (static_cast<ReMaterial*>(matNode->pointer)->getName() == materialName) {
            node->removeChild(i);
            break;
          }
        }
      }
    }
  }
};

/**
 Class that implements a Qt Data model that accesses the objects and materials
 stored. This is used for the main Tree View of the Reality Material Editor.
 */

class ReSceneDataModel : public QAbstractItemModel {
  Q_OBJECT
  
private:

  RealityDataRelay* dataServerConnector;

  int nodeIsObject;
  int nodeIsMaterial;

  TreeItem* rootNode;

  QIcon eyeOpenIcon,
        eyeClosedIcon;
  // Array to list how we align each column. With this
  //  we avoid expensive "if" tests. See <data()>
  int columnAlignement[6];

public:
  // Constructor: ReSceneDataModel
  ReSceneDataModel(QObject* parent = 0);

  // Destructor: ReSceneDataModel
  ~ReSceneDataModel();

  int rowCount(const QModelIndex& index) const;
  int columnCount(const QModelIndex& parent ) const;
  
  void setSceneData( RealityDataRelay* _data );

  /**
   Sends the request to the server to fetch an object by its ID. The response
   is handled by the <getObject> method, which is connected by the \sa setSceneData()
   method.
   */
  inline void addObject( const QString& objectID ) {
    // First retrieve the list of volumes, they might have changed and we 
    // need to make sure that the volumes are up to date before we receive
    // the materials.
    dataServerConnector->sendMessageToServer(GET_VOLUMES);    

    // Retrieve the object
    QVariantMap args;
    args["objectID"] = objectID;
    dataServerConnector->sendMessageToServer(GET_OBJECT, &args);
  }

  /**
   * When a material is updated in the host app and Reality is running we
   * remove it first and then request the new version of the same material
   * which will be added again when the <dataServerConnector> object calls
   * us back. The method that responds to the "call" is \sa updatedMaterialReady()
   */
  void updateMaterial( const QString objectID, 
                       const QString materialName, 
                       const bool isLightMaterial = false);

  /**
   Deletes an object from the scene. This is in response to the action of the user
   in the hosting app.
   */
  void deleteObject( const QString& objectID );

  //! Returns the model index for a match based on object id and material id.
  QModelIndex findMaterial( const QString& objectID, const QString& matID );

  //! Returns the pointer to the node that holds a reference to the object 
  //! with name objectID or NULL if the object is not in the model.
  TreeItem* findObject( const QString& objectID );

  /**
   * Let the view know that we use checkboxes for our items
   */
  Qt::ItemFlags flags ( const QModelIndex& index ) const;

  /**
   * Provides the data for the view based on our database of objects 
   * and materials.
   */ 
  QVariant data( const QModelIndex& index, int role ) const;

  /**
   * This method is reimplemented only to provide the functionality of 
   * checking/unchecking the visibility of a material in the render
   */
  bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

  ReGeometryObject* getSelectedObject(const QModelIndex& index);

  ReMaterial* getSelectedMaterial( const QModelIndex& index );

  QModelIndex index( int row, int column, const QModelIndex& parentIndex ) const;

  QModelIndex parent( const QModelIndex& index ) const;

  QVariant headerData(int section,Qt::Orientation orientation, int role) const;

  inline void refreshRow( const QModelIndex& row ) {
    emit dataChanged(row, row);
  }

  inline void refresh() {
    reset();
  }

  void setNewScene();

public slots:
  //! Method called in response to an object being added to the scene.
  //! The object is added to the ReSceneData object and the model is 
  //! updated as well.
  void addObjectToModel(ReGeometryObject* newObj, 
                        const bool resetModeFlag = true);

  //! Method called in response to the event of adding a whole list of 
  //! objects to the scene. It called the addObjectToModel() method in
  //! turn.
  void addObjectsToModel( ReGeometryObjectDictionary list );

  /**
   *   This method responds to the last part of the sequence of updating a
   *   material. The old version of the material has been removed at this point
   *   and the new version has been installed in the object. All we need to do
   *   is to create a <TreeItem> node to connect to the new version of the material
   */
  void updatedMaterialReady( const QString objectID, const QString matName );

signals:
  //! Emitted when the visibility of the material is changed
  void materialUpdated(QString objectID, QString matName, QString valueName, QVariant value);

  //! Emitted when the type of the material changes 
  void materialTypeChanged(const QString& objectID, const QString& materialName);
};


} // namespace

#endif