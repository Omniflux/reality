/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MAT_ACTIONS_H
#define RE_MAT_ACTIONS_H

#include <QtCore>
#include <QUndoCommand>
#include "ReSceneDataGlobal.h"
#include "RealityBase.h"
#include "RealityDataRelay.h"
#include "ReMaterials.h"

using namespace Reality;

class ChangeMaterialTypeAction : public QUndoCommand {

private:
  QString objectID, materialID;
  ReMaterialType newType,originalMatType;
public:

  ChangeMaterialTypeAction( const QString& objectID, 
                            const QString& materialID, 
                            const ReMaterialType newType ) : 
    objectID(objectID), 
    materialID(materialID),
    newType(newType) {

    originalMatType = RealitySceneData->getMaterial(objectID, materialID)->getType();
    setText(QString(QObject::tr("Convert to %1")).arg(ReMaterial::getTypeAsString(newType)));

  }

  void redo() {
    QVariantMap args;
    args["objectID"] = objectID;
    args["matID"]    = materialID;
    args["newType"]  = newType;
    realityDataRelay->sendMessageToServer(CHANGE_MATERIAL_TYPE, &args);
    ReMaterialType tmp = originalMatType;
    originalMatType  = newType;
    newType = tmp;
  }

  void undo() {
    // We constantly flip-flop between the originalType and newType materials    
    redo();
  }
};

#endif
