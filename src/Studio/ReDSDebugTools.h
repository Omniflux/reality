/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include <QtCore>
#include "dzobject.h"
#include "dzshape.h"
#include "dzmaterial.h"

/**
  Class: ReDSDebugTool
 */

class ReDSDebugTool : public QObject {
  Q_OBJECT

private:
  static ReDSDebugTool* instance;

  //! Constructor: ReDSDebugTool
  ReDSDebugTool() {
  };

  static void init();
public:

  static ReDSDebugTool* getInstance();

private slots:
  // DzObject debugging
  void currentShapeSwitchedHandler();
  void drawnDataChangedHandler();
  void materialListChangedHandler();
  void materialSelectionChangedHandler();
  void modifierAddedHandler(DzModifier* md);
  void modifierRemovedHandler(DzModifier* md);
  void modifierStackChangedHandler();
  void shapeAddedHandler(DzShape* shape);
  void shapeRemovedHandler(DzShape* shape);
  void uvsChangedHandler();

  // DzShape signal handlers
  void shapeGeomChangedHandler();
  void shapeMaterialAddedHandler(DzMaterial* mat);
  void shapeMaterialChangedHandler();
  void shapeMaterialListChangedHandler();
  void shapeMaterialRemovedHandler(DzMaterial* mat);
  void shapeMaterialSelectedHandler(DzMaterial* mat);
  void shapeMaterialSelectionChangedHandler();
  void shapeMaterialUnselectedHandler(DzMaterial* mat);
  void shapeRigidityGroupListChangedHandler();
  void shapeRigidityMapChangedHandler();
  void shapeSmoothingChangedHandler();
  void shapeUvsChangedHandler();

public slots:
  void monitorObject( DzObject* obj );
  void monitorShape( DzShape* shape );
  
};


inline void dsMonitorObject( DzObject* obj ) {
  ReDSDebugTool::getInstance()->monitorObject(obj);
}

inline void dsMonitorShape( DzShape* shape ) {
  ReDSDebugTool::getInstance()->monitorShape(shape);
}
