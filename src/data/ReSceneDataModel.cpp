/**
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
*/

#include "ReSceneDataModel.h"

#include "ReSceneDataGlobal.h"


namespace Reality {

ReSceneDataModel::ReSceneDataModel( QObject* parent ) : QAbstractItemModel(parent) {
  dataServerConnector = 0;
  nodeIsMaterial = 1001;
  nodeIsObject   = 1002;

  // Setting up the alignment values for each column;
  columnAlignement[RE_MB_COL_MATERIAL] = 
  columnAlignement[RE_MB_COL_TYPE] = 
  columnAlignement[RE_MB_COL_EMITTER] = Qt::AlignLeft;
  
  columnAlignement[RE_MB_COL_EDITED] = 
  columnAlignement[RE_MB_COL_HIDDEN] = Qt::AlignHCenter;
  columnAlignement[RE_MB_COL_ACSEL] = Qt::AlignLeft;

  // Initialize the tree
  rootNode = new TreeItem(TreeItem::NodeIsObject, NULL);
};

// Destructor: ReSceneDataModel
ReSceneDataModel::~ReSceneDataModel() {
  delete rootNode;
};


int ReSceneDataModel::rowCount(const QModelIndex& index) const {
  TreeItem* parentNode;
  if (index.column() > 0) {
    return 0;
  }
  
  if (!index.isValid()) {
    parentNode = rootNode;
  }
  else {
    parentNode = static_cast<TreeItem*>(index.internalPointer());
  }
  if (!parentNode) {
    return 0;
  }
  return parentNode->getNumChildren();
}

int ReSceneDataModel::columnCount(const QModelIndex& parent ) const {
  // Columns: name, type, isEdited, Emitter, ACSEL
  return RE_MB_NUM_COLUMS;
}


void ReSceneDataModel::setSceneData( RealityDataRelay* _data ) {
  dataServerConnector = _data;

  // Setting up the receiver for the message from the socket. This message
  // is received when all the objects in the scene have been transferred 
  // from the server-side part of Reality
  connect(dataServerConnector, SIGNAL(objectsReady(ReGeometryObjectDictionary)),
          this,                SLOT(addObjectsToModel(ReGeometryObjectDictionary)));

  // This message is received when the GUI is already running and the user
  // adds a new object to the host (Poser or Studio)
  connect(dataServerConnector, SIGNAL(objectReady(ReGeometryObject*)),
          this,                SLOT(addObjectToModel(ReGeometryObject*)));

  connect(dataServerConnector, SIGNAL(materialReady(const QString, const QString)),
          this,                SLOT(updatedMaterialReady(const QString, const QString)));

}

void ReSceneDataModel::updateMaterial( const QString objectID, 
                                       const QString materialName, 
                                       const bool isLightMaterial ) 
{
  rootNode->removeMaterial(objectID, materialName);
  QVariantMap args;
  args["objectID"] = objectID;
  args["materialName"] = materialName;
  dataServerConnector->sendMessageToServer(GET_MATERIAL, &args);
  // Request the updated list of lights. This changes if the material
  // ha been changed to light and vice versa
  dataServerConnector->sendMessageToServer(GET_OBJECT_LIGHTS, &args);
}

void ReSceneDataModel::deleteObject( const QString& objectID ) {
  beginResetModel();
  TreeItem* theNode; 
  bool found = false;
  int index;
  int numChildren = rootNode->children.count();
  for (index = 0; index < numChildren; ++index) {
    theNode = rootNode->children[index];
    if (theNode->nodeType == TreeItem::NodeIsObject) {
      ReGeometryObject* obj = static_cast<ReGeometryObject*>(theNode->pointer);
      if (!obj) {
        continue;
      }
      if (obj->getInternalName() == objectID ) {
        found = true;
        break;
      }
    }
  }
  if (found) {
    delete theNode;
    rootNode->children.removeAt(index);
    RealitySceneData->deleteObject(objectID);
  }
  endResetModel();
}

QModelIndex ReSceneDataModel::findMaterial( const QString& objectID, 
                                            const QString& matID ) 
{
  if (!rootNode) {
    RE_LOG_DEBUG() <<  "Error: rootNode of the data model is NULL";
    return createIndex(0, 0);
  }
  TreeItem* theNode; 
  bool found = false;
  int index = 0;
  int numChildren = rootNode->getNumChildren();
  for (index = 0; index < numChildren; index++) {
    theNode = rootNode->children[index];
    if (theNode->nodeType == TreeItem::NodeIsObject) {
      ReGeometryObject* obj = static_cast<ReGeometryObject*>(theNode->pointer);
      if (obj->getInternalName() == objectID ) {
        found = true;
        break;
      }
    }
  }
  if (found) {
    int numMats = theNode->children.count();
    for (int i = 0; i < numMats; i++) {
      ReMaterial* mat = static_cast<ReMaterial*>(theNode->children[i]->pointer);
      if (mat->getName() == matID) {
        return createIndex(i, 0, theNode);
      }
    }
  }
  return QModelIndex();
}

TreeItem* ReSceneDataModel::findObject( const QString& objectID ) {
  if (!rootNode) {
    RE_LOG_DEBUG() <<  "Error: rootNode of the data model is NULL";
    return NULL;
  }
  TreeItem* theNode; 
  int index = 0;
  int numChildren = rootNode->getNumChildren();
  for (index = 0; index < numChildren; index++) {
    theNode = rootNode->children[index];
    if (theNode->nodeType == TreeItem::NodeIsObject) {
      ReGeometryObject* obj = static_cast<ReGeometryObject*>(theNode->pointer);
      if (obj->getInternalName() == objectID ) {
        return theNode;
        break;
      }
    }
  }
  return NULL;
}

Qt::ItemFlags ReSceneDataModel::flags( const QModelIndex& index ) const {
  if (!index.isValid()) {
      return 0;
  }
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  // Signal that the Hidden column is checkable
  if ( index.column() == RE_MB_COL_HIDDEN && item->nodeType == TreeItem::NodeIsMaterial) {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
}

ReGeometryObject* ReSceneDataModel::getSelectedObject(const QModelIndex& index) {
  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  
  void* obj = NULL;
  if (item && item->nodeType == TreeItem::NodeIsObject) {
    obj = item->pointer;
  }
  else {
    auto parent = item->getParent();
    if (parent) {
      obj = parent->pointer;
    }
  }

  return static_cast<ReGeometryObject*>(obj);
}

ReMaterial* ReSceneDataModel::getSelectedMaterial( const QModelIndex& index ) {
  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  
  ReMaterial* mat = NULL;
  if (item->nodeType == TreeItem::NodeIsMaterial) {
    mat = static_cast<ReMaterial*>(item->pointer);
  }
  return (mat);    
}

QModelIndex ReSceneDataModel::index( int row, int column, const QModelIndex& parentIndex ) const {
  // Invalid index, no value
  if (!hasIndex(row,column, parentIndex)) {  
    return QModelIndex();
  }
  
  TreeItem* parentItem;
  if ( !parentIndex.isValid() ) {
    parentItem = rootNode;
  }
  else {
    parentItem = static_cast<TreeItem*>(parentIndex.internalPointer());
  }
  
  TreeItem* childNode = parentItem->getChild(row);
  
  if ( childNode ) {
    return createIndex(row, column, childNode);
  }
  else {
    return QModelIndex();
  }
}

QModelIndex ReSceneDataModel::parent( const QModelIndex& index ) const {
  if (!index.isValid()) {
    return QModelIndex();
  }

  TreeItem* childNode = static_cast<TreeItem*>(index.internalPointer());
  TreeItem* parentNode = childNode->getParent();
  
  if (parentNode == rootNode) {
    return QModelIndex();
  }

  // if (parentNode == NULL) {
  //   return createIndex(0, 0, rootNode);
  // }
  return createIndex(parentNode->getRow(), 0, parentNode);
}

QVariant ReSceneDataModel::headerData(int section,Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    switch(section) {
      case RE_MB_COL_MATERIAL:
        return tr("Object/Material");
      case RE_MB_COL_TYPE:
        return tr("Type");
      case RE_MB_COL_EDITED:
        return tr("Edited");
      case RE_MB_COL_HIDDEN:
        return tr("Visible");
      case RE_MB_COL_EMITTER:
        return tr("Emitter");
      case RE_MB_COL_ACSEL:
        return tr("Preset");
    }
  }
  return QVariant();
}

QVariant ReSceneDataModel::data( const QModelIndex& index, int role ) const {
  if (!index.isValid()) {
    return QVariant();
  }
  int column = index.column();

  if (role == Qt::TextAlignmentRole) {
    // Align the "Edited" column as centered
    return QVariant(columnAlignement[column]);
  }

  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  ReMaterial* mat = NULL;
  if (item->nodeType == TreeItem::NodeIsMaterial) {
    mat = static_cast<ReMaterial*>(item->pointer);
  }
  if (role == Qt::DisplayRole) {
    if (item->nodeType == TreeItem::NodeIsObject) {
      if (column == RE_MB_COL_MATERIAL) {
        auto itmPtr = static_cast<ReGeometryObject*>(item->pointer);
        if (itmPtr) {
          return itmPtr->getName();
        }
      }
      return QVariant();
    }
    if (!mat) {
      return QVariant();
    }
    switch(column) {
      case RE_MB_COL_MATERIAL:
        return mat->getName();
      case RE_MB_COL_TYPE:
        return mat->getTypeAsString();
      case RE_MB_COL_EDITED:
        return (mat->isEdited() ? "Y" : "");
      case RE_MB_COL_HIDDEN:
        return "";
      case RE_MB_COL_EMITTER: {
        ReModifiedMaterial* dmat = dynamic_cast<ReModifiedMaterial*>(mat);
        if (dmat && dmat->isEmittingLight()) {
          return "Y";
        }
        return "";
      }
      case RE_MB_COL_ACSEL:
        return mat->getAcselSetName();
    }
  }
  if ( role == Qt::CheckStateRole && 
       item->nodeType == TreeItem::NodeIsMaterial &&
       column == RE_MB_COL_HIDDEN ) {
    return static_cast< int >( mat->isVisibleInRender() ? Qt::Unchecked : Qt::Checked );
  }

  return QVariant();
}

bool ReSceneDataModel::setData( const QModelIndex& index, 
                                const QVariant& value, 
                                int role ) {
  if (!index.isValid()) {
    return false;
  }

  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  ReMaterial* mat = NULL;
  if (item->nodeType == TreeItem::NodeIsMaterial) {
    mat = static_cast<ReMaterial*>(item->pointer);
  }
  else {
    return false;
  }

  if (index.column() == RE_MB_COL_HIDDEN) {
    if (!mat) {
      return false;
    }
    mat->setVisibleInRender(!mat->isVisibleInRender());
    emit dataChanged(index, index);

    emit materialUpdated(
      mat->getParent()->getInternalName(),
      mat->getName(),
      "visibleInRender",
      mat->isVisibleInRender()
    );
    return true;
  }
  return false;
}

void ReSceneDataModel::setNewScene() {
  beginResetModel();

  delete rootNode;

  // Initialize the tree
  rootNode = new TreeItem(TreeItem::NodeIsObject, NULL);

  // Send the message to retrieve the objects in the scene
  dataServerConnector->sendMessageToServer(GET_OBJECTS);

  // Request the list of Lights to the server
  dataServerConnector->sendMessageToServer(GET_LIGHTS);  

  // Request the list of cameras to the server
  dataServerConnector->sendMessageToServer(GET_CAMERAS);

  // Retrieves the library paths set by the host-app
  dataServerConnector->sendMessageToServer(GET_LIBRARY_PATHS);

  // Retrieve the list of volumes
  dataServerConnector->sendMessageToServer(GET_VOLUMES);    
  endResetModel();
}

void ReSceneDataModel::addObjectToModel(ReGeometryObject* newObj,
                                        const bool resetModelFlag )  
{
  RealitySceneData->addObject(newObj);
  // Skip mesh lights
  if (newObj->isLight()) {
    return;
  }
  if (resetModelFlag) {
    beginResetModel();
  }

  TreeItem* item = new TreeItem(TreeItem::NodeIsObject,(void *) newObj, rootNode);
  rootNode->addChild(item);
  
  QHashIterator<QString, ReMaterialPtr> i(newObj->getMaterials());
  while (i.hasNext()) {
    i.next();
    // Skip Light materials
    if (i.value()->getType() == MatLight) {
      continue;
    }
    TreeItem* matItem = new TreeItem(TreeItem::NodeIsMaterial,
                                     (void*) i.value().data(),
                                     item);
    item->addChild(matItem);
  }
  if (resetModelFlag) {  
    endResetModel();
  }
}

void ReSceneDataModel::addObjectsToModel(ReGeometryObjectDictionary list) {
  beginResetModel();
  delete rootNode;
  rootNode = new TreeItem(TreeItem::NodeIsObject, NULL);
  
  RealitySceneData->setObjects(list);
  QHashIterator<QString, ReGeometryObjectPtr> i(list);
  while(i.hasNext()) {
    i.next();
    addObjectToModel(i.value().data(), false);
  }
  endResetModel();
}

void ReSceneDataModel::updatedMaterialReady( const QString objectID, const QString matName ) {
  // Scan the tree finding all the parent nodes which point to objects
  TreeItem* node;
  TreeItem* matItem;
  bool added = false;
  foreach( node, rootNode->children ) {
    ReGeometryObject* obj = static_cast<ReGeometryObject*>(node->pointer);
    if (obj->getInternalName() == objectID ) {
      auto mat = obj->getMaterial(matName).data();
      if (mat->getType() == MatLight) {
        continue;
      }
      matItem = new TreeItem( TreeItem::NodeIsMaterial, (void*) mat, node );
      node->addChild(matItem);
      added = true;
      break;
    }
  }
  if (added) {
    QModelIndex matIndex = createIndex(matItem->getRow(), 0, matItem->parent); //findMaterial(objectID, matName);
    emit dataChanged(matIndex, matIndex);      
    emit materialTypeChanged(objectID, matName);
  }
}


} // namespace
