/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#include "RealityDataRelay.h"
#include "ReIPC.h"
#include "ReLogger.h"
#include "ReSceneDataGlobal.h"

namespace Reality {

// This is initilized by <realitypanel.cpp> when the GUI start up
  
RealityDataRelay* realityDataRelay = NULL;
  
void RealityDataRelay::stop() {
  QMutexLocker locker(&lock);
  isActive = false;
}

void RealityDataRelay::sendMessageToServer( const IPC_MESSAGES msg, 
                                            const QVariantMap* args ) {
  
  // Send the message by converting the message code to a quint16 to determine the
  // size of the message code. This avoid ambiguities about how big the data is.
  QByteArray command;
  QDataStream commandStream(&command, QIODevice::WriteOnly);
  commandStream << (quint16) msg;

  /*
   The following commands are the ones that have parameters. 
   The switch statement extracts the parameters as they are 
   expected to be received and it copies them into the data stream
   */
  switch(msg) {
    case GET_MATERIAL:
    case SELECT_MATERIAL_IN_HOST: {
      commandStream << args->value("objectID").toString()
                    << args->value("materialName").toString();
      break;
    }
    case CHANGE_MATERIAL_TYPE: {
      commandStream << args->value("objectID").toString()
                    << args->value("matID").toString()
                    << args->value("newType").toInt();
      break;
    }
    case MATERIAL_IMPORT_FROMCLIPBOARD_DATA: {
      commandStream << args->value("objectID").toString()
                    << args->value("materialName").toString()
                    << args->value("data").toString()
                    << (quint16)args->value("replaceTextures").toInt();
      break;
    }
    case CHANGE_TEXTURE_TYPE: {
      commandStream << args->value("objectID").toString()
                    << args->value("materialName").toString()
                    << args->value("textureName").toString()
                    << args->value("newType").toInt();
      break;
    }
    case GET_OBJECT: {
      commandStream << args->value("objectID").toString();        
      break;
    }
    case GET_OBJECT_LIGHTS: {
      commandStream << args->value("objectID").toString();
      break;
    }
    case UI_MATERIAL_EDITED: {
      if (args->contains("textureName")) {
        commandStream << QString("setTexture") 
                      << args->value("objectID").toString()
                      << args->value("materialName").toString()
                      << args->value("propertyName").toString()
                      << args->value("value")         
                      << args->value("textureName").toString();
      }
      else {
        commandStream << QString("setMaterial") 
                      << args->value("objectID").toString()
                      << args->value("materialName").toString()
                      << args->value("propertyName").toString()
                      << args->value("value");          
      }
      break;
    }
    case MAKE_NEW_TEXTURE: {
      commandStream << args->value("objectID").toString() 
                    << args->value("materialName").toString() 
                    << args->value("channelName").toString() 
                    << (quint16) args->value("textureType").toInt()
                    << (quint16) args->value("dataType").toInt();

      break;        
    }
    case MAKE_NEW_SUB_TEXTURE: {
      commandStream << args->value("objectID").toString() 
                    << args->value("materialName").toString() 
                    << args->value("channelName").toString() 
                    << args->value("masterTextureName").toString() 
                    << args->value("newTextureName").toString() 
                    << (quint16) args->value("textureType").toInt()
                    << (quint16) args->value("dataType").toInt();

      break;        
    }
    case LINK_TEXTURE: {
      commandStream << args->value("objectID").toString() 
                    << args->value("materialName").toString() 
                    << args->value("channelName").toString() 
                    << args->value("textureName").toString();
      break;        
    }
    case LINK_SUB_TEXTURE: {
      commandStream << args->value("objectID").toString() 
                    << args->value("materialName").toString() 
                    << args->value("channelName").toString() 
                    << args->value("textureName").toString()
                    << args->value("subTextureName").toString();
      break;        
    }
    case UNLINK_SUB_TEXTURE: {
      commandStream << args->value("objectID").toString() 
                    << args->value("materialName").toString() 
                    << args->value("textureName").toString()
                    << args->value("channelName").toString();
      break;        
    }
    case UNLINK_TEXTURE: {
      commandStream << args->value("objectID").toString() 
                    << args->value("materialName").toString() 
                    << args->value("channelName").toString();
      break;        
    }
    case SYNCHRONIZE_CHANNEL_TEXTURE: {
      commandStream << args->value("objectID").toString() 
                    << args->value("sourceMaterialName").toString() 
                    << args->value("channelName").toString()
                    << args->value("targetMaterialName").toString();
      break;        
    }
    case REVERT_SHADER_TO_ORIGINAL: {
      commandStream << args->value("objectID").toString()
                    << args->value("materialName").toString();
      break;
    }
    case UI_LIGHT_EDITED: {
      commandStream << args->value("lightID").toString()
                    << args->value("valueName").toString()
                    << args->value("value");
      break;
    }      
    case UI_CAMERA_EDITED: {
      commandStream << args->value("cameraID").toString()
                    << args->value("valueName").toString()
                    << args->value("value");
      break;
    }      

    case VOLUME_ADDED: 
    case VOLUME_UPDATED: {
      ReVolumePtr vol = RealitySceneData->getVolume(args->value("volumeName").toString());
      vol->serialize(commandStream);
      break;
    }
    case VOLUME_DELETED: {
      commandStream << args->value("volumeName").toString();
      break;
    }
    case VOLUME_RENAMED: {
      commandStream << args->value("oldVolumeName").toString()
                    << args->value("newVolumeName").toString();
      break;
    }
    case SYNC_OUTPUT_DATA: {
      QByteArray opts;
      RealitySceneData->getRenderOptions(opts);
      // commandStream << opts;
      shmChannel.write(ReSharedMemIPC::FixedSizeChannel, opts);
      break;
    }
    case UI_CAMERA_SELECTED: {
      commandStream << args->value("value").toString();
      break;
    }
    case RENDER_FRAME: {
      commandStream << args->value("runRenderer").toBool();
      break;
    }
    case RENDER_ANIMATION: {
      commandStream << args->value("runRenderer").toBool()
                    << args->value("startFrame").toInt()
                    << args->value("endFrame").toInt();
      break;
    }
    case SET_IBL_PREVIEW: {
      commandStream << args->value("previewMap").toString();
      break;
    }
    case APPLY_ACSEL_SHADER: {
      commandStream << args->value("objectID").toString()
                    << args->value("matID").toString()
                    << args->value("setID").toString();
      break;
    }
    case APPLY_UNIVERSAL_SHADER: {
      commandStream << args->value("objectID").toString()
                    << args->value("matID").toString()
                    << args->value("shaderID").toString();
      break;
    }
    case REPLACE_TEXTURE: {
      commandStream << args->value("objectID").toString()
                    << args->value("materialName").toString()
                    << args->value("channelName").toString()
                    << args->value("textureData").toString()
                    << args->value("masterTextureName").toString();
      break;
    }
    default:
      break;
  }

  try {
    zmq::message_t message(command.size());
    memcpy((void *)message.data(), command.data(), command.size());    
    if ( !serverCommandInterface->send(message) ) {
      RE_LOG_WARN() << "Could not send the message " << command.data()
                    << " to the host application";
      return;
    }
  }
  catch( zmq::error_t e ) {
    RE_LOG_WARN() << QString("ZMQ send exception: %1").arg(e.what()).toStdString();    
  }

  try {
    // Wait for a reply. There is always one, even when no real data
    // is returned. This keeps the protocol simple.
    // To avoid blocking the thread if the server goes off-line we check
    // the availability of the server and, if necessary, we retry three times
    // before giving up.
    int retries_left = 3;
    zmq::pollitem_t items[] = { { *serverCommandInterface, 0, ZMQ_POLLIN, 0 } };

    while (retries_left) {
      // Poll socket for a reply, with timeout
      // The timeout is in milliseconds...
      zmq::poll (&items[0], 1, 100);

      //  If we got a reply, process it
      if (items[0].revents & ZMQ_POLLIN) {
        break;
      }

      retries_left--;
    }
    // If there is no answer from the server then delete the socket to clear
    // all pending requests, recreate the socket and get out of this method.
    if (retries_left == 0) {
      delete serverCommandInterface;
      createDataServerSendingSocket();
      return;
    }
  }
  catch( zmq::error_t e ) {
    RE_LOG_WARN() << QString("ZMQ recv exception: %1").arg(e.what()).toStdString();
    // Delete the socket to clear all pending requests, recreate the socket 
    // and get out of this method.
    delete serverCommandInterface;
    createDataServerSendingSocket();
    return;
  }

# if IPC_DEBUG
    RE_LOG_INFO() << "Reading the reply from the data server";
# endif

  // At this point it's safe to read from the server
  zmq::message_t reply;
  serverCommandInterface->recv(&reply);

# if IPC_DEBUG
  RE_LOG_INFO() << QString(
    "Received %1 bytes from the server."
  ).arg(reply.size()).toStdString();
# endif

  QByteArray buffer((char *)reply.data(),reply.size());
  QDataStream dataStream(&buffer, QIODevice::ReadOnly);
  // The following is really not necessary but we do it anyway...
  dataStream.setVersion(QDataStream::Qt_4_6);

  // Retrieve the command connected with the information that we are receiving.
  // This ensures that we have the context right without having to rely on a 
  // specific state, which would be an unreliable way of interpreting information
  quint16 serverCmd;
  dataStream >> serverCmd;

# if IPC_DEBUG
  RE_LOG_INFO() << QString("  msg code: %1.").arg(serverCmd).toStdString();
# endif

  switch(serverCmd){
    case GET_OBJECT: {
      auto newObj = ReGeometryObject::deserialize(dataStream);
      emit objectReady(newObj);
      if (newObj->isLightEmitter()) {
        emit lightsReady();
      }
      break;
    }
    case GET_OBJECT_LIGHTS: {
      QString objectID;
      dataStream >> objectID;

      ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
      if (!obj.isNull()) {
        obj->deserializeMaterialLights(dataStream);
      }
      break;
    }
    case GET_OBJECTS: {
      qint32 numObjects;      
      dataStream >> numObjects;
      ReGeometryObjectDictionary objDict;
      for (int i = 0; i < numObjects; ++i) {
        auto newObj = ReGeometryObject::deserialize(dataStream);
        objDict[newObj->getInternalName()] = ReGeometryObjectPtr(newObj);
      }
      emit objectsReady(objDict);
      break;
    }
    case GET_OBJECT_NAMES: {
      QStringList names;
      dataStream >> names;
      emit objectNamesReady(names);
      break;
    }
    case GET_MATERIAL: {
      QString objectName, materialName;
      dataStream >> objectName >> materialName;
      ReGeometryObjectPtr obj = RealitySceneData->getObject(objectName);
      if (!obj.isNull()) {
        obj->deserializeMaterial(materialName, dataStream);
        emit materialReady(objectName, materialName);
      }
      break;
    }
    case GET_VOLUMES: {
      quint16 numVolumes;
      dataStream >> numVolumes;

      for (int i = 0; i < numVolumes; ++i) {
        ReVolumePtr vol = ReVolumePtr(new ReVolume());
        vol->deserialize(dataStream);
        RealitySceneData->saveVolume(vol);
      }
      emit volumesTransferred();
      break;
    }
    case GET_NUM_OBJECTS: {
      int numObjects;
      dataStream >> numObjects;
      emit numObjectsReady(numObjects);
      break;
    }
    case GET_CAMERAS: {
      RealitySceneData->removeAllCameras();
      qint16 numCameras;
      dataStream >> numCameras;
      for (int i = 0; i < numCameras; ++i) {
        ReCameraPtr cam = ReCameraPtr( new ReCamera("temp", "temp"));
        cam->deserialize(dataStream);
        RealitySceneData->addCamera(cam);
      }
      // Retrieve the name of the selected camera
      QString selectedCamera;
      dataStream >> selectedCamera;
      RealitySceneData->selectCamera(selectedCamera);
      emit camerasReady();
      break;
    }
    case GET_LIGHTS: {
      RealitySceneData->removeAllLights();
      qint16 numLights;
      dataStream >> numLights;
      QString lightID;
      for (int i = 0; i < numLights; ++i) {
        dataStream >> lightID;
        ReLightPtr light = ReLightPtr( new ReLight("", "") );
        light->deserialize(dataStream);
        RealitySceneData->saveLight(lightID, light);
      }
      emit lightsReady();
      break;
    }
    case GET_OUTPUT_DATA: {
      QByteArray newData;
      shmChannel.read(ReSharedMemIPC::FixedSizeChannel, newData);
      // dataStream >> newData;
      RealitySceneData->setRenderOptions(newData);
      emit outputDataReady();
      break;
    }
    case GET_LIBRARY_PATHS: {
      QStringList paths;
      dataStream >> paths;
      if (paths.count() > 0) {
        RealityBase::getRealityBase()->setLibraryPaths(paths);
      }
      break;
    }
    case GUI_CONNECTED: {
      // The backend answers by sending the ID, a string, of the host application
      QString hostAppID;
      dataStream >> hostAppID;
      RealityBase::getRealityBase()->setHostAppID(hostAppID);
      emit hostAppIDReady();
      break;
    }
    case GUI_DISCONNECTED: {
      QString ack;
      dataStream >> ack;
      break;
    }
    default:
      break;
  }
}

// This method handles the notification messages received from the 
// host-app side.
void RealityDataRelay::forwardNotification( zmq::message_t& update ) {

  QByteArray buffer((char *)update.data(),update.size());
  QDataStream dataStream(&buffer, QIODevice::ReadOnly);
  // The following is really not necessary but we do it anyway...
  dataStream.setVersion(QDataStream::Qt_4_6);

  quint16 msg;

  dataStream >> msg;
#if IPC_DEBUG
  RE_LOG_INFO() << QString(
    "Received notification from host-app: %1"
  ).arg(msg).toStdString();
#endif

  switch(msg) {
    case GUI_BRING_TO_FRONT: {
      emit bringUIToFront();
      break;
    }
    case GUI_PAUSE_MAT_PREVIEW: {
      emit pauseMaterialPreview(true);
      break;
    }
    case GUI_RESUME_MAT_PREVIEW: {
      emit pauseMaterialPreview(false);
      break;
    }
    case HOST_SCENE_LOADED: {
      emit sceneLoaded();
      break;
    }
    case HOST_OBJECT_ADDED: {
      QString objName;
      dataStream >> objName;
      emit objectAdded(objName);
      break;
    }
    case HOST_OBJECT_DELETED: {
      QString objName;
      dataStream >> objName;
      emit objectDeleted(objName);    
      break;
    }
    case HOST_OBJECT_RENAMED: {
      QString objectID,newName;
      dataStream >> objectID >> newName;
      emit objectRenamed(objectID, newName);
      break;
    }  
    case HOST_OBJECT_ID_RENAMED: {
      QString objectID,newID;
      dataStream >> objectID >> newID;
      emit objectIDRenamed(objectID, newID);
      break;
    }
    case HOST_OBJECT_VISIBILITY_CHANGED: {
      QString objectID;
      bool isVisible;
      dataStream >> objectID >> isVisible;
      emit objectVisibilityChanged(objectID, isVisible);
      break;
    }
    case HOST_MATERIAL_UPDATED: {
      QString materialName,objectID;
      dataStream >> materialName >> objectID;
      emit materialUpdated(materialName,objectID);
      break;
    }
    case HOST_MATERIAL_TYPE_CHANGED: {
      QString materialName,objectID;
      quint16 newTypeInt;
      dataStream  >> objectID >> materialName >> newTypeInt;
      emit materialTypeChanged(objectID, materialName, newTypeInt);
      break;
    }
    case HOST_MATERIAL_SELECTED: {
      QString objectID, matID;
      dataStream >> objectID >> matID;

      emit materialSelected(objectID, matID);
      break;
    }
    case HOST_LIGHT_ADDED: {
      QString lightID;
      dataStream >> lightID;
      ReLightPtr light = ReLightPtr( new ReLight("xxxx", lightID) );
      light->deserialize(dataStream);
      RealitySceneData->saveLight(lightID, light);
      emit lightAdded(lightID);
      break;
    }
    case HOST_LIGHT_DELETED: {
      QString lightID;
      dataStream >> lightID;
      RealitySceneData->deleteLight(lightID);
      emit lightDeleted(lightID);
      break;
    }  
    case HOST_LIGHTS_REMOVE_ALL: {
      RealitySceneData->removeAllLights();
      emit allLightsRemoved();
      break;
    }  
    case HOST_LIGHT_RENAMED: {
      QString lightID;
      QString newName;
      dataStream >> lightID >> newName;
      emit lightRenamed(lightID, newName);
      break;
    }
    case HOST_LIGHT_TYPE_CHANGED: {
      QString lightID;
      dataStream >> lightID;
      emit lightTypeChanged(lightID);
      break;
    }
    case HOST_LIGHT_UPDATED: {
      QString lightID, lightName;
      dataStream >> lightID >> lightName;
      ReLightPtr light = RealitySceneData->getLight(lightID); 
      light->deserialize(dataStream);
      // RealitySceneData->saveLight(lightID, light);
      emit lightUpdated(lightID);
      break;
    }
    case HOST_CAMERA_ADDED: {
      QString cameraID;
      dataStream >> cameraID;
      emit cameraAdded(cameraID);
      break;
    }  
    case HOST_CAMERA_RENAMED: {
      QString objectID,newName;
      dataStream >> objectID >> newName;
      emit cameraRenamed(objectID, newName);
      break;
    }  
    case HOST_CAMERA_DELETED: {
      QString objectID,newName;
      dataStream >> objectID;
      emit cameraRemoved(objectID);
      break;
    }  
    case HOST_CAMERA_SELECTED: {
      QString camID;
      dataStream >> camID;
      RealitySceneData->selectCamera(camID);
      emit hostCameraSelected(camID);
      break;
    }  

    // This message is received when the camera is modified
    // in the host app. Cameras get updated from both the host app
    // and the Reality GUI at the same time. This precludes the possibility
    // to do a full replacement to synchronize the camera data. 
    // On the other hand, the set of data changed by each side is completely
    // isolated. The GUI is only concerned about changing the ISO, shutter
    // speed, f-stop and so on. These are fields that are not changed by 
    // the host-app side. The same is valid in the other direction so there
    // is no need to synch the data, but simply to transfer the portion of
    // the camera data that each side affects. In the following code we 
    // simply take the data from the host-app:
    //   - The transformation matrix
    //   - The focal length
    //   - The focal distance
    case HOST_CAMERA_CHANGED: {
      QString cameraID;
      dataStream >> cameraID;
      ReCameraPtr cam = RealitySceneData->getCamera(cameraID);
      if (!cam) {
        cam = ReCameraPtr(new ReCamera(cameraID,0));
      }
      ReCameraPtr tempCam = ReCameraPtr( new ReCamera("tempcam", "tempcam") );
      tempCam->deserialize(dataStream);
      cam->fromHostAppCamera(tempCam);
      emit cameraChanged(cameraID);
      break;
    }
    case REFRESH_CAMERA_LIST: {
      emit camerasReady();
      break;
    }

    case HOST_FRAME_SIZE_UPDATED : {
      quint32 width, height;
      dataStream >> width >> height;
      RealitySceneData->setFrameSize(width, height);

      emit frameSizeChanged();      
      break;
    }
    case EXP_EXPORT_STARTED: {
      int numObjects;
      dataStream >> numObjects;
      emit exportStarted(numObjects);
      break;
    }  
    case EXP_EXPORTING_OBJECT: {
      QString objName;
      dataStream >> objName;
      emit exportingObject(objName);
      break;
    }  
    case EXP_EXPORT_FINISHED: {
      emit exportFinished();
      break;
    }  
    case RENDER_DIMENSIONS_NOT_SET: {
      emit renderDimensionsNotSet();
      break;
    }  
    case UPDATE_ANIMATION_LIMITS: {
      int startFrame, endFrame, fps;
      dataStream >> startFrame >> endFrame >> fps;
      RealitySceneData->setAnimationLimits(startFrame, endFrame, fps);
      break;
    }
    case SET_NEW_SCENE: {
      emit sceneHasBeenReset();
      break;
    }
    case CLOSE_REALITY: {
    # if IPC_DEBUG
      RE_LOG_INFO() << "Asked to close the GUI";
    # endif
      emit close();
      break;
    }
  }
}

void RealityDataRelay::run() {
  RE_LOG_INFO() << " Started UI message relayer...";
  createDataServerSendingSocket();

  // In order to receive notifications of relevant events from the hosted plug-in,
  // we need to use another socket as a channel of information. If an object is 
  // added to the scene, for example, when the GUI is running, the hosted plugin
  // will notify the GUI via this socket.
  // The connection is of type PUBlisher/SUBscriber. In this way the messages move
  // only from the hosted side toward the GUI, with no need for reply.
  zmq::socket_t guiCommandInterface(ipcContext, ZMQ_SUB);

  try {
    QString subscriberAddress = QString(IPC_CLIENT_ADDRESS).arg(serverAddress).arg(ipcPubSubPort);
    guiCommandInterface.connect(subscriberAddress.toAscii().data());
    guiCommandInterface.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
    
    // Set the option for the socket to avoid lingering when closing the program
    // This makes it possible to shutdown the GUI even if the server (Poser/Studio)
    // is not running anymore.
    int linger = 0;
    guiCommandInterface.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));    
  }
  catch(zmq::error_t e) {
    RE_LOG_INFO() << QString(
      "Error: initialization of the IPC thread: %1").arg(e.what()).toStdString();
  }

  isActive = true;
  emit upAndRunning();

  while(true) {
    {
      QMutexLocker locker(&lock);
      if (!isActive) {
        break;
      }
    }
    try {
      zmq::message_t update;
      guiCommandInterface.recv(&update);
      forwardNotification(update);
    }
    catch( zmq::error_t e ) {
      RE_LOG_INFO() << QString(
        "Error in data relayer main loop: %1").arg(e.what()).toStdString();
    }

  }
}

} // namespace
