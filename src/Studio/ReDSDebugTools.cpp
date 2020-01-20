#include "ReDSDebugTools.h"
#include "ReLogger.h"

#include "dznode.h"

ReDSDebugTool* ReDSDebugTool::instance = NULL;

ReDSDebugTool* ReDSDebugTool::getInstance() {
  return instance;
}

void ReDSDebugTool::monitorObject( DzObject* obj ) {
  connect(obj, SIGNAL(currentShapeSwitched()), this, SLOT(currentShapeSwitchedHandler));
  connect(obj, SIGNAL(drawnDataChanged()), this, SLOT(drawnDataChangedHanlder));
  connect(obj, SIGNAL(materialListChanged()), this, SLOT(materialListChangedHandler));
  connect(obj, SIGNAL(materialSelectionChanged()), this, SLOT(materialSelectionChangedHandler));
  connect(obj, SIGNAL(modifierAdded(DzModifier* )), this, SLOT(modifierAddedHandler(DzModifier*)));
  connect(obj, SIGNAL(modifierRemoved(DzModifier*)), this, SLOT(modifierRemovedHandler(DzModifier*)));
  connect(obj, SIGNAL(modifierStackChanged()), this, SLOT(modifierStackChangedHandler()));
  connect(obj, SIGNAL(shapeAdded(DzShape*)), this, SLOT(shapeAddedHandler(DzShape*)));
  connect(obj, SIGNAL(shapeRemoved(DzShape*)), this, SLOT(shapeRemovedHandler(DzShape*)));
  connect(obj, SIGNAL(uvsChanged()), this, SLOT(uvsChangedHandler()));
  RE_LOG_INFO() << "Added monitoring of object " << obj->getName().toStdString();
}

void ReDSDebugTool::init() {
  instance = new ReDSDebugTool();
  RE_LOG_INFO() << "DS Debug tool initialized";
}


void ReDSDebugTool::currentShapeSwitchedHandler() {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "1]! Current shape switched for object " << source->getName().toStdString();
};

void ReDSDebugTool::drawnDataChangedHandler() {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "2]! Draw data changed for " << source->getName().toStdString();
};

void ReDSDebugTool::materialListChangedHandler() {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "3]! Material list changed for " << source->getName().toStdString();
};

void ReDSDebugTool::materialSelectionChangedHandler() {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "4]! Material selection changed for " << source->getName().toStdString();
};

void ReDSDebugTool::modifierAddedHandler(DzModifier* md) {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "5]! Modifier added for object " << source->getName().toStdString();
};

void ReDSDebugTool::modifierRemovedHandler(DzModifier* md) {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "6]! Modifier removed for object " << source->getName().toStdString();
};

void ReDSDebugTool::modifierStackChangedHandler() {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "7]! Modifier stack changed for object " << source->getName().toStdString();
};

void ReDSDebugTool::shapeAddedHandler(DzShape* shape) {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "8]! Shape added to object " << source->getName().toStdString();
};

void ReDSDebugTool::shapeRemovedHandler(DzShape* shape) {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "9]! Shape removed from object " << source->getName().toStdString();
};

void ReDSDebugTool::uvsChangedHandler() {
  auto source = qobject_cast<DzObject*>(QObject::sender());
  RE_LOG_INFO() << "10]! UVs changed on object " << source->getName().toStdString();
};


void ReDSDebugTool::shapeGeomChangedHandler() {
  auto source = qobject_cast<DzShape*>(QObject::sender());
  RE_LOG_INFO() << "11]! Geometry for shape changed on node " 
                << source->getNode()->getName().toStdString();
};
void ReDSDebugTool::shapeMaterialAddedHandler(DzMaterial *mat) {
  auto source = qobject_cast<DzShape*>(QObject::sender());
  RE_LOG_INFO() << QString("12]! Material %1 added to shape for node %2")
                     .arg(mat->getName())
                     .arg(source->getNode()->getName())
                     .toStdString();
};
void ReDSDebugTool::shapeMaterialChangedHandler() {
  RE_LOG_INFO() << "13]! ";
};
void ReDSDebugTool::shapeMaterialListChangedHandler() {
  auto source = qobject_cast<DzShape*>(QObject::sender());
  RE_LOG_INFO() << QString("14]! Material list changed for %1")
                     .arg(source->getNode()->getName())
                     .toStdString();
};
void ReDSDebugTool::shapeMaterialRemovedHandler(DzMaterial *mat) {
  RE_LOG_INFO() << "15]! ";
};
void ReDSDebugTool::shapeMaterialSelectedHandler(DzMaterial *mat) {
  RE_LOG_INFO() << "16]! ";
};
void ReDSDebugTool::shapeMaterialSelectionChangedHandler() {
  RE_LOG_INFO() << "17]! ";
};
void ReDSDebugTool::shapeMaterialUnselectedHandler(DzMaterial *mat) {
  RE_LOG_INFO() << "18]! ";
};
void ReDSDebugTool::shapeRigidityGroupListChangedHandler() {
  RE_LOG_INFO() << "19]! ";
};
void ReDSDebugTool::shapeRigidityMapChangedHandler() {
  RE_LOG_INFO() << "20]! ";
};
void ReDSDebugTool::shapeSmoothingChangedHandler() {
  RE_LOG_INFO() << "21]! ";
};
void ReDSDebugTool::shapeUvsChangedHandler() {
  RE_LOG_INFO() << "22]! ";
};


void ReDSDebugTool::monitorShape( DzShape* shape ) {
  connect(shape, SIGNAL(geomChanged()), this, SLOT(shapeGeomChangedHandler()));
  connect(shape, SIGNAL(materialAdded(DzMaterial*)), this, SLOT(shapeMaterialAddedHandler(DzMaterial*)));
  connect(shape, SIGNAL(materialChanged()), this, SLOT(shapeMaterialChangedHandler()));
  connect(shape, SIGNAL(materialListChanged()), this, SLOT(shapeMaterialListChangedHandler()));
  connect(shape, SIGNAL(materialRemoved(DzMaterial*)), this, SLOT(shapeMaterialRemovedHandler(DzMaterial*)));
  connect(shape, SIGNAL(materialSelected(DzMaterial*)), this, SLOT(shapeMaterialSelectedHandler(DzMaterial*)));
  connect(shape, SIGNAL(materialSelectionChanged()), this, SLOT(shapeMaterialSelectionChangedHandler()));
  connect(shape, SIGNAL(materialUnselected(DzMaterial*)), this, SLOT(shapeMaterialUnselectedHandler(DzMaterial*)));
  connect(shape, SIGNAL(rigidityGroupListChanged()), this, SLOT(shapeRigidityGroupListChangedHandler()));
  connect(shape, SIGNAL(rigidityMapChanged()), this, SLOT(shapeRigidityMapChangedHandler()));
  connect(shape, SIGNAL(smoothingChanged()), this, SLOT(shapeSmoothingChangedHandler()));
  connect(shape, SIGNAL(uvsChanged()), this, SLOT(shapeUvsChangedHandler()));

  RE_LOG_INFO() << "Added monitoring of shape for " << shape->getNode()->getName().toStdString();
}

