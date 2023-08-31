/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wswitch"
#endif

#include "ReDSStorage.h"

#include <boost/any.hpp>
#include <dzscene.h>
#include <idzjsonio.h>

#include "Reality_DS.h"
#include "ReLogger.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"


namespace Reality {

// forward declaration
void saveListToJsonIO( const QVariantList& list, IDzJsonIO* io );

void saveMapToJsonIO( const QVariantMap& map, IDzJsonIO* io ) {
  QMapIterator<QString, QVariant> i(map);

  while( i.hasNext() ) {
    i.next();
    QString key = i.key();
    QVariant val = i.value();

    switch(val.type()) {
      case QMetaType::QString:
        io->addMember(key, val.toString());
        break;
      // DSON cannot differentiate between ints and floats so
      // it writes ints as: <key> : [ "i", <value> ] 
      // which is quite ugly and makes things unnecessary complicated.
      // So, Reality exports ints to be floats/double as well.
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::Long:
      case QMetaType::ULong:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Float:
      case QMetaType::Double:
        io->addMember(key, val.toDouble());
        break;
      case QMetaType::Bool:
        io->addMember(key, val.toBool());
        break;
      case QMetaType::QVariantMap: {
        io->startMemberObject(key);
        saveMapToJsonIO(val.toMap(), io);
        io->finishObject();
        break;
      }
      case QMetaType::QVariantList:
        io->startMemberArray(key);
        saveListToJsonIO(val.toList(), io);
        io->finishArray();
        break;
    }
  }
};

void saveListToJsonIO( const QVariantList& list, IDzJsonIO* io ) {
  int count = list.count();
  for (int i = 0; i < count; i++) {
    QVariant val = list[i];

    switch(val.type()) {
      case QMetaType::QString:
        io->addItem(val.toString());
        break;
      // DSON cannot differentiate between ints and floats so
      // it writes ints as: <key> : [ "i", <value> ] 
      // which is quite ugly and makes things unnecessary complicated.
      // So, Reality exports ints to be floats/double as well.
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::Long:
      case QMetaType::ULong:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Float:
      case QMetaType::Double:
        io->addItem(val.toDouble());
        break;
        break;
      case QMetaType::Bool:
        io->addItem(val.toBool());
        break;
      case QMetaType::QVariantMap: {
        io->startObject();
        saveMapToJsonIO(val.toMap(), io);
        io->finishObject();
        break;
      }
      case QMetaType::QVariantList:
        io->startArray();
        saveListToJsonIO(val.toList(), io);
        io->finishArray();
        break;
    }
  }
};

//! Return the embedded instance created by \ref Reality_DS::addSceneStorage
DzSceneData* ReStorage::createDataItem( const DzFileIOSettings *opts ) const {
  return dzScene->findDataItem(RE_DS_SCENEDATA_ID);
};

bool ReStorage::shouldWrite( QObject* object, const DzFileIOSettings* opts ) const {
  return RealitySceneData->doesNeedsSaving();
};

DzError ReStorage::writeExtraInstance( QObject* object, 
                                       IDzJsonIO* io, 
                                       const DzFileIOSettings* opts ) const
{
  auto exporter = RealitySceneData->getSceneExporter("map");
  boost::any sceneData;
  exporter->exportScene(0, sceneData);
  QVariantMap qSceneData = boost::any_cast<QVariantMap>(sceneData);
  saveMapToJsonIO(qSceneData, io);
  return DZ_NO_ERROR;
};

// This is where Studio starts reading the JSOn data for the Reality scene
DzAssetJsonObject* ReStorage::startInstanceRead( DzAssetJsonItem* parentItem ) {
  Reality_DS::enableNodeAddition(false);
  
  Reality_DS::updateLibraryPaths();

  sceneContext = new ReSceneContext(parentItem->getFile());
  return new ReSceneReader(sceneContext);
};


DzError ReStorage::applyInstanceToObject( QObject* object, const DzFileIOSettings* opts ) const {
  auto sceneData = sceneContext->getData();

  // When loading a scene in merge mode, File | Merge from DS, we can have potential
  // conflicts with objects that are already in the scene. This can happen if the 
  // user merges the same scene over and over again. In that case we need to 
  // rename the conflicting objects, lights and cameras by obtaining a new unique
  // identifier.
  //
  // The following typedef and QMap implement a list of objects to be renamed. 
  // The key is the original key in the map and the value is a pair containing, 
  // respectively, the new ID and the new label.
  typedef std::pair<QString, QString> renamePair;
  QMap< QString, renamePair> newIDs;
  
  // The saving of the patch number in the scene data was started after
  // beta 1 of Reality 4.1. If it doesn't exist in the data loaded then
  // this scene will need to convert the IDs for the cameras and lights.
  bool sceneNeedsConversion = !sceneData.contains(KEY_SCENE_PATCH_VERSION);

  // Check if we are loading an instance of an object that is already in the 
  // scene. This can happen when using the File | Merge option of DS
  QVariantMap objs = sceneData.value("objects").toMap();
  QMapIterator<QString, QVariant> oi(objs);
  bool inMerge = Reality_DS::getInstance()->isSceneMerging();

  while( oi.hasNext() ) {
    oi.next();
    QString objID = oi.key();
    if (RealitySceneData->hasObject(objID)) {
      QString newID, newLabel;
      if ( Reality_DS::getInstance()->calcIDForObject(objID, newID, newLabel) ) {
        newIDs[objID] = renamePair(newID, newLabel);        
      }
    }
  }
  QMapIterator<QString, renamePair> ri(newIDs);
  while(ri.hasNext()) {
    ri.next();
    QString oldKey = ri.key();
    auto newIDs = ri.value();
    auto objData = objs.value(oldKey).toMap();
    objData["internalName"] = newIDs.first;
    objData["name"] = newIDs.second;
    objs.remove(oldKey);
    objs[newIDs.first] = objData;
  }
  
  sceneData["objects"] = objs;

  // Now let's do the same for lights
  newIDs.clear();
  QVariantMap lights = sceneData.value("lights").toMap();
  oi = lights;

  auto RealityPlugin = Reality_DS::getInstance();

  // Then let's check if the lights already are present in the scene. If so
  // then we need to change the ID and the label
  while( oi.hasNext() ) {
    oi.next();
    QString lightID = oi.key();
    // If we are merging scenes and the light is IBL then we delete it and 
    // move on, we don't want to erase the data from the existing IBL light
    // already in the scene
    if (inMerge && lightID == "IBL") {
      RE_LOG_INFO() << "Skipping IBL data when merging scenes";
      lights.remove(lightID);
      continue;
    }

    // If we load scene data saved before Reality 4.1 then the lights used the
    // DzNode::getName() value for the ID. That value is, unfortunately, not 
    // unique and so it cannot be used reliably. Starting with Reality 4.1 we 
    // generate our own unique ID. So, for lights loaded from an older scene
    // we need to generate a new unique ID.
    //
    // At the same time a merging operation can bring a light, which has
    // identifier of a light already in the scene, and so we need to create a 
    // new GUID. 
    //
    // Lastly there can be a case of a light that has different GUID of any 
    // light already in the scene but it has the same label, in the file, of 
    // a light already in the scene. In that case Studio renames the light's
    // label to avoid confusion. If we simply restore the light from the data
    // saved by Reality then the label will be wrong. By executing the 
    // calcIDForObject() method we obtain the new label even in the case that the
    // GUID has not changed
    if ( lightID != "IBL" ) {
      QString newID, newLabel;
      if ( RealityPlugin->calcIDForObject(lightID, newID, newLabel) ) {
        newIDs[lightID] = renamePair(newID, newLabel);
      }
    }
  }

  ri = newIDs;
  while(ri.hasNext()) {
    ri.next();
    QString oldKey = ri.key();
    auto newIDs = ri.value();
  
    auto lightData = lights.value(oldKey).toMap();
    lightData["id"] = newIDs.first;
    lightData["name"] = newIDs.second;
    lights.remove(oldKey);
    lights[newIDs.first] = lightData;
  }
  sceneData["lights"] = lights;

  // Now let's do that for the cameras
  newIDs.clear();
  QVariantMap cameras = sceneData.value("cameras").toMap();
  oi = cameras;

  // Then let's check if the cameras already are present in the scene. If so
  // then we need to change the ID and the label
  while( oi.hasNext() ) {
    oi.next();
    QString camID = oi.key();

    // If we load scene data saved before Reality 4.1 then the cameras used the
    // DzNode::getAssetId() value for the ID. That value is, unfortunately, not 
    // unique and so it cannot be used reliably. Starting with Reality 4.1 we 
    // generate our own unique ID. So, for cameras loaded from an older scene
    // we need to generate a new unique ID.
    if (sceneNeedsConversion || RealitySceneData->hasCamera(camID)) {
      QString newID, newLabel;
      if ( RealityPlugin->calcIDForObject(camID, newID, newLabel) ) {
        newIDs[camID] = renamePair(newID, newLabel);
      }
    }
  }

  ri = newIDs;
  while(ri.hasNext()) {
    ri.next();
    QString oldKey = ri.key();
    auto newIDs = ri.value();
  
    auto camData = cameras.value(oldKey).toMap();
    camData["id"] = newIDs.first;
    camData["name"] = newIDs.second;
    cameras.remove(oldKey);
    cameras[newIDs.first] = camData;
  }
  sceneData["cameras"] = cameras;

  // Restore the scene data
  RealitySceneData->restoreScene(sceneData, inMerge);
  Reality_DS::enableNodeAddition(true);
  return DZ_NO_ERROR;
};

DzError ReStorage::resolveInstance( QObject* object, const DzFileIOSettings* opts ) const {
  return DZ_NO_ERROR;
};

bool ReSceneReader::addMember( const QString& name, const QString& val ) {
  sceneContext->setValue(name, QVariant(val));
  return true;
};

bool ReSceneReader::addMember( const QString &name, double val ) {
  int intVal = static_cast<int>(val);
  bool isInteger = static_cast<double>(intVal) == val;
  if (isInteger) {
    sceneContext->setValue( name,QVariant(intVal) );
  }
  else {
    sceneContext->setValue( name, QVariant(val) );
  }
  return true;
};

bool ReSceneReader::addMember( const QString &name, bool val ) {
  sceneContext->setValue( name, QVariant(val) );
  return true;
};

DzAssetJsonItem* ReSceneReader::startMemberObject( const QString& name ) {
  sceneContext->startObject(name);
  return new ReSceneReader(sceneContext);
};

DzAssetJsonItem* ReSceneReader::startMemberArray( const QString& name ) {
  sceneContext->startList(name);
  return new ReSceneReader(sceneContext);
};

bool ReSceneReader::addItem( const QString& val ) {
  sceneContext->setValue(val);
  return true;
};

bool ReSceneReader::addItem( double val ) {
  sceneContext->setValue(val);
  return true;
};

bool ReSceneReader::addItem( bool val ) {
  sceneContext->setValue(val);
  return true;
};


void ReSceneReader::finishObject( DzAssetJsonItem *item ) {
  sceneContext->endObject();
};

void ReSceneReader::finishArray( DzAssetJsonItem *item ) {
  sceneContext->endList();
};

DzAssetJsonItem* ReSceneReader::startObject() {
  sceneContext->startObject();  
  return new ReSceneReader(sceneContext);
};

DzAssetJsonItem* ReSceneReader::startArray() {
  sceneContext->startList();
  return new ReSceneReader(sceneContext);
};

} // namespace
