/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

/**
 File: ReIPC.cpp

 This file contains only the global pointer to the IPC object used by the Reality backend to communicate
 with its front-end.   
 */
#include <QJson/Parser>

#include "ReIPC.h"
#include "textures/ReComplexTexture.h"
#include "ReTextureCreator.h"
#include "importers/qt/ReQtMaterialImporterFactory.h"
#include "ReSceneDataGlobal.h"
#include "zeromqTools.h"
#include "RealityRunner.h"
#include "ReAcsel.h"

namespace Reality {

/**
 * The "event loop" IPC thread that polls the commands from the GUI
 */
CommandPollingThread* realityIPC;

void CommandPollingThread::setSceneAsDirty() {
  RealityBase* rb = RealityBase::getRealityBase();
  rb->commandStackPush("changed");
  RealitySceneData->setNeedsSaving(true);
}

CommandPollingThread::CommandPollingThread( const HostAppID appID ) : 
  dataStream(&dataBuffer, QIODevice::WriteOnly),
  ipcContext(1),
  appID(appID)
{
  isActive        = false;
  guiAppIsRunning = false;
  notifySocket    = NULL;
  // The following is really not necessary but we do it anyway...
  dataStream.setVersion(QDataStream::Qt_4_6);
  lastGUIPing = 0;
}

CommandPollingThread::~CommandPollingThread() {
  if (notifySocket) {
    notifySocket->close();
    delete notifySocket;
  }
}

void CommandPollingThread::transmit( QByteArray& buffer ) {
  quint32 dataLenght = buffer.length();
  zmq::message_t message(dataLenght);
  memcpy((void *) message.data (), buffer.data(), dataLenght);
  notifySocket->send(message);    
}

void CommandPollingThread::sendReplyToGUI( zmq::socket_t& socket, 
                                           const IPC_MESSAGES cmd, 
                                           const QString str) 
{
  dataBuffer.clear();
  dataStream.device()->seek(0);
  // QDataStream dataStream(&dataBuffer, QIODevice::WriteOnly);
  dataStream << (quint16) cmd << str;

  quint32 dataLenght = dataBuffer.length();
  zmq::message_t reply(dataLenght);
  memcpy((void *) reply.data (), dataBuffer.data(), dataLenght);
  socket.send(reply);
}

void CommandPollingThread::run() {
  isActive = true;

  int ipcPort,
      ipcPubSubPort;
  switch(appID) {
    case Poser:
      ipcPort = IPC_PORT;
      ipcPubSubPort = IPC_PUBSUB_PORT;
      break;
    case DAZStudio:
      ipcPort = IPC_DS_PORT;
      ipcPubSubPort = IPC_DS_PUBSUB_PORT;
      break;
    case RealityPro:
      ipcPort = IPC_PRO_PORT;
      ipcPubSubPort = IPC_PRO_PUBSUB_PORT;
      break;
    default:
      ipcPort = IPC_PORT;
      ipcPubSubPort = IPC_PUBSUB_PORT;
  }

  QString serverAddress = QString(IPC_SERVER_ADDRESS).arg(ipcPort);
  zmq::socket_t socket(ipcContext, ZMQ_REP);
  int hwm = 3000;
  socket.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(int));

  try {
    // This is the REQuest/REPlay socket used to received commands from the
    // GUI. We provide requested data through this channel.
    socket.bind(serverAddress.toAscii().data());
  }
  catch( ... ) {
    return;
  }
  // This is the PUBlish/SUBcribe socket used to notify the GUI of events happening
  // in the hosting application. Typical notifications are: HOST_OBJECT_ADDED, HOST_OBJECT_RENAMED
  // etc. This side of the transaction is the PUBlisher.
  QString publisherAddress = QString(IPC_PUBLISHER).arg(ipcPubSubPort);
  notifySocket = new zmq::socket_t(ipcContext, ZMQ_PUB);
  int linger = 0;
  notifySocket->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));    

  notifySocket->bind(publisherAddress.toAscii().data());

  while( true ) {
    {
      QMutexLocker locker(&threadRunningFlagLock);
      if (!isActive) {
        break;
      }
    }
    try {
      zmq::message_t request;

      // If there is no message then go back to the begnning of the loop
      // so that we have the chance to close the thread if requested.
      if (!zmqHasMessages(socket)) {
        continue;
      }

      //  Wait for next request from client
      socket.recv(&request);
      processRequest(request,socket);
    }
    catch( zmq::error_t e ) {
//#     if IPC_DEBUG
        RE_LOG_DEBUG() << QString("Exception in the data server: %1")
                            .arg(e.what());
//#     endif
    }
  }
  RE_LOG_INFO() << "Message queue stopped";
}

/**
 Processes the requests from the GUI app and delivers the data requested.
*/
#define RESET_IPC_BUFFER  dataBuffer.clear(); dataStream.device()->seek(0);

void CommandPollingThread::processRequest(zmq::message_t& request, zmq::socket_t& socket) {
  QByteArray commandBuffer((char *)request.data(), static_cast<int>(request.size()));
  QDataStream commandStream(&commandBuffer, QIODevice::ReadOnly);
  quint16 code;
  commandStream >> code;

  IPC_MESSAGES cmd = (IPC_MESSAGES)code;
# if IPC_DEBUG
    RE_LOG_DEBUG() << QString("Received remote command: *%1*").arg(cmd);
# endif
  bool doWrite = false;

  try {
    switch(cmd) {
      case GET_NUM_OBJECTS: {
        sendReplyToGUI(socket, cmd, RealitySceneData->getNumObjects());        
        break;
      }
      case GET_OBJECT_NAMES: {
        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                

        dataStream << RealitySceneData->getObjects().keys();
        doWrite = true;
        break;
      }
      case GET_OBJECT: {
        QString objectName;
        commandStream >> objectName;

        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                

        ReGeometryObjectPtr obj = RealitySceneData->getObject(objectName);
        obj->serialize(dataStream);
        doWrite = true;
        break;
      }
      case GET_OBJECT_LIGHTS: {
        QString objectID;
        commandStream >> objectID;

        RESET_IPC_BUFFER
        dataStream << (quint16) cmd << objectID;                

        ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
        obj->serializeMaterialLights(dataStream);

        doWrite = true;
        break;
      }
      case GET_OBJECTS: {
        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                
        /*
         GeometryObject:

         This object database is streamed with this format:

         start(code)
         [numobjects] [sequence of objects]
         end(code)

         A sequence of objects is streamed as:

         start(code)

         [object name] [number of materials] [material name] ...

         end(code)

         */
        
        // Number of objects
        dataStream << (qint32)RealitySceneData->getNumObjects();

        ReGeometryObjectIterator i(RealitySceneData->getObjects());
        while( i.hasNext() ) {
          i.next();
          ReGeometryObjectPtr obj = i.value();
          // Object data
          obj->serialize(dataStream);
        }
        doWrite = true;
        break;
      }
      case GET_MATERIAL: {
        RESET_IPC_BUFFER

        QString objectName, materialName;
        commandStream >> objectName >> materialName;
        ReMaterialPtr mat = RealitySceneData->getMaterial(objectName, materialName);
        if (!mat.isNull()) {
          dataStream << (quint16) cmd << objectName << materialName;
          mat->serialize(dataStream);
        }
        doWrite = true;          
        break;
      } 
      case CHANGE_MATERIAL_TYPE: {
        RESET_IPC_BUFFER
        
        QString objectID, materialID;
        int newType;

        commandStream >> objectID >> materialID >> newType;  
        sendReplyToGUI(socket, cmd, "OK");
        setSceneAsDirty();
        changeMaterialType(
          objectID, materialID, static_cast<ReMaterialType>(newType)
        );
        
        break;
      }
      case MATERIAL_IMPORT_FROMCLIPBOARD_DATA: {
        RESET_IPC_BUFFER
        
        QString objectID, materialID, clipboardData;
        quint16 replaceTexturesInt;

        commandStream >> objectID >> materialID 
                      >> clipboardData >> replaceTexturesInt;
        sendReplyToGUI(socket, cmd, "OK");
        
        auto replaceTextures = static_cast<ReQtMaterialImporter::ReplaceTextureMode>(
                                 replaceTexturesInt
                               );
        updateMatFromClipboardData(
          objectID, materialID, clipboardData, replaceTextures
        );
        break;
      }
      case CHANGE_TEXTURE_TYPE: {
        RESET_IPC_BUFFER
        
        QString objectID, materialID, textureName;
        int newType;
        commandStream >> objectID >> materialID >> textureName >> newType;
        auto mat = RealitySceneData->getObject(objectID)->getMaterial(materialID);
        mat->changeTextureType(textureName, 
                               static_cast<ReTextureType>(newType));
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case APPLY_ACSEL_SHADER: {
        RESET_IPC_BUFFER

        QString objectName, materialName, setID;
        commandStream >> objectName >> materialName >> setID;
        sendReplyToGUI(socket, cmd, "OK");

        applyAcselShader(objectName, materialName, setID);
        break;
      }
      case APPLY_UNIVERSAL_SHADER: {
        RESET_IPC_BUFFER

        QString objectName, 
                materialName,
                shaderID;
        commandStream >> objectName >> materialName >> shaderID;
        sendReplyToGUI(socket, cmd, "OK");

        applyUniversalShader(objectName, materialName, shaderID);
        break;
      }
      case REVERT_SHADER_TO_ORIGINAL: {
        RESET_IPC_BUFFER

        QString objectName, materialName;
        commandStream >> objectName >> materialName;
        sendReplyToGUI(socket, cmd, "OK");

        revertToOriginalShader(objectName, materialName);
        break;
      }
      /*
       Serialization of volumes
       */
      case GET_VOLUMES: {
        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                

        ReVolumeDictionary volumes = RealitySceneData->getVolumes();
        dataStream << (quint16) volumes.count();
        ReVolumeIterator i(volumes);
        while( i.hasNext() ) {
          i.next();
          i.value()->serialize(dataStream);
        }
        doWrite = true;
        break;
      }        

      case GET_LIGHTS: {
        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                

        dataStream << (qint16)RealitySceneData->getNumLights();
        ReLightIterator i(RealitySceneData->getLights());
        while( i.hasNext() ) {
          i.next();
          dataStream << i.key();
          ReLightPtr light = i.value();
          if (light.isNull()) {
            continue;
          }
          light->serialize(dataStream);
        }
        doWrite = true;
        break;
      }

      case GET_CAMERAS: {
        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                

        dataStream << (qint16)RealitySceneData->getNumCameras();
        ReCameraDictionary* cameras = RealitySceneData->getCameras();
        ReCameraIterator i(*cameras);
        while( i.hasNext() ) {
          i.next();
          // dataStream << i.key() << i.value()->getName() << i.value()->getFocalLength();
          i.value()->serialize(dataStream);
        }
        // Send the name of the selected camera, or NONE if no camera
        // has been seelcted
        QString selectedCamera = RealitySceneData->getSelectedCameraID();
        if (selectedCamera.isEmpty()) {
          selectedCamera = "NONE";
        }
        dataStream << selectedCamera;

        doWrite = true;
        break;
      }
      case UI_CAMERA_SELECTED: {
        QString cameraID;
        commandStream >> cameraID;
        RealitySceneData->selectCamera(cameraID);
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case UI_MATERIAL_EDITED: {
        QString commandType,
                objectID,
                materialName,
                textureName,
                propertyName;
        QVariant value;

        commandStream >> commandType >> objectID 
                      >> materialName >> propertyName 
                      >> value;

        if (commandType == "setTexture") {
          commandStream >> textureName;
        }
        uiMaterialEdited(objectID, materialName, propertyName, value, textureName);           
        sendReplyToGUI(socket, cmd, "OK");
        setSceneAsDirty();
        
        break;
      }
      case MAKE_NEW_TEXTURE: {
        QString objectID, 
                materialID, 
                channelName;
        quint16 textureTypeInt, dataTypeInt;
        commandStream >> objectID >> materialID 
                      >> channelName >> textureTypeInt 
                      >> dataTypeInt;
        ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
        if (!obj.isNull()) {
          ReMaterialPtr mat = obj->getMaterial(materialID);
          if (!mat.isNull()) {
            mat->makeNewTexture( channelName, 
                                 static_cast<ReTextureType>(textureTypeInt),
                                 static_cast<ReTexture::ReTextureDataType>(dataTypeInt));
          }
        }
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case MAKE_NEW_SUB_TEXTURE: {
        QString objectID, 
                materialName, 
                channelName,
                masterTextureName,
                newTextureName;
        quint16 textureTypeInt, dataTypeInt;

        commandStream >> objectID >> materialName 
                      >> channelName >> masterTextureName
                      >> newTextureName >> textureTypeInt 
                      >> dataTypeInt;
        makeNewSubTexture( objectID, 
                           materialName, 
                           masterTextureName, 
                           newTextureName, 
                           channelName, 
                           static_cast<ReTextureType>(textureTypeInt), 
                           static_cast<ReTexture::ReTextureDataType>(dataTypeInt) );
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }        
      case LINK_TEXTURE: {
        QString objectID, materialID, channelName, textureName;
        commandStream >> objectID >> materialID >> channelName >> textureName;
        ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
        if (!obj.isNull()) {
          ReMaterialPtr mat = obj->getMaterial(materialID);
          if (!mat.isNull()) {
            mat->setChannel(channelName, textureName);
          }
        }
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case LINK_SUB_TEXTURE: {
        QString objectID, 
                materialID, 
                channelName,
                textureName,
                subTextureName;

        commandStream >> objectID >> materialID 
                      >> channelName >> textureName
                      >> subTextureName;

        linkSubTexture( objectID, materialID, channelName, textureName, subTextureName );
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case UNLINK_SUB_TEXTURE: {
        QString objectID, 
                materialID, 
                textureName,
                channelName;

        commandStream >> objectID    >> materialID 
                      >> textureName >> channelName;

        unlinkSubTexture( objectID, materialID, textureName, channelName );
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case UNLINK_TEXTURE: {
        QString objectID, materiaName, channelName;
        commandStream >> objectID >> materiaName >> channelName;
        ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
        if (!obj.isNull()) {
          ReMaterialPtr mat = obj->getMaterial(materiaName);
          if (!mat.isNull()) {
            mat->setChannel(channelName, "");
          }
        }
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case REPLACE_TEXTURE: {
        QString objectID,
                materialName,
                channelName,
                jsonTextureData,
                masterTextureName;
        commandStream >> objectID >> materialName 
                      >> channelName >> jsonTextureData
                      >> masterTextureName;
        replaceTexture(objectID, materialName, channelName, 
                       jsonTextureData, masterTextureName);
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        break;
      }
      case UI_LIGHT_EDITED: {
        QString lightID,
                valueName;
        QVariant value;

        commandStream >> lightID >> valueName >> value;
        ReLightPtr light = RealitySceneData->getLight(lightID);
        if (!light.isNull()) {
          light->setNamedValue(valueName, value);
        }

        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case SYNCHRONIZE_CHANNEL_TEXTURE: {
        QString objectID, 
                sourceMaterialName, 
                channelName,
                targetMaterialName,
                targetChannel;

        commandStream >> objectID    >> sourceMaterialName 
                      >> channelName >> targetMaterialName;
        synchronizeChannelTexture(
          objectID, sourceMaterialName, channelName, targetMaterialName
        );
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }

      case UI_CAMERA_EDITED: {
        QString cameraID,
                valueName;
        QVariant value;

        commandStream >> cameraID >> valueName >> value;
        ReCameraPtr camera = RealitySceneData->getCamera(cameraID);
        if (!camera.isNull()) {
          camera->setNamedValue(valueName, value);
        }

        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case VOLUME_ADDED:
      case VOLUME_UPDATED: {
        ReVolumePtr vol = ReVolumePtr( new ReVolume() );
        vol->deserialize(commandStream);
        RealitySceneData->saveVolume(vol);
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case VOLUME_DELETED: {
        QString volName;
        commandStream >> volName;
        RealitySceneData->removeVolume(volName);
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case VOLUME_RENAMED: {
        QString oldName, newName;
        commandStream >> oldName >> newName;
        RealitySceneData->renameVolume(oldName, newName);
        setSceneAsDirty();
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case GET_OUTPUT_DATA: {
        QByteArray renderOptions;
        RealitySceneData->getRenderOptions(renderOptions);
        shmChannel.write(ReSharedMemIPC::FixedSizeChannel, renderOptions);
        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                
        // dataStream << renderOptions;
        doWrite = true;
        break;
      }
      case SYNC_OUTPUT_DATA: {
        QByteArray newData;
        shmChannel.read(ReSharedMemIPC::FixedSizeChannel, newData);
        // commandStream >> newData;
        sendReplyToGUI(socket, cmd, "OK");
        setSceneAsDirty();
        RealitySceneData->setRenderOptions(newData);
        
        break;
      }
      case GUI_CONNECTED: {
        sendReplyToGUI(socket, cmd, RealityBase::getRealityBase()->getHostAppIDAsString());
        RealitySceneData->setNeedsSaving(true);
        guiAppIsRunning = true;
        
        break;
      }
      case GUI_DISCONNECTED: {
        sendReplyToGUI(socket, cmd, "bye");
        guiAppIsRunning = false;
        
        break;
      }
      case RENDER_FRAME: {
        bool runRenderer;
        commandStream >> runRenderer;
        RealityBase* rb = RealityBase::getRealityBase();
        rb->commandStackPush(runRenderer ? "1" : "0");
        rb->commandStackPush("render");
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case RENDER_ANIMATION: {
        bool runRenderer;
        int startFrame, endFrame;
        commandStream >> runRenderer >> startFrame >> endFrame;
        RealityBase* rb = RealityBase::getRealityBase();
        rb->commandStackPush(QString::number(endFrame));
        rb->commandStackPush(QString::number(startFrame));
        rb->commandStackPush(runRenderer ? "1" : "0");
        rb->commandStackPush("renderAnim");
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case SAVE_SCENE: {
        RealityBase* rb = RealityBase::getRealityBase();
        rb->commandStackPush("save");
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case REFRESH_SCENE: {
        RealityBase* rb = RealityBase::getRealityBase();
        rb->commandStackPush("refresh");
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case GET_LIBRARY_PATHS: {
        RESET_IPC_BUFFER
        dataStream << (quint16) cmd;                
        dataStream << RealityBase::getRealityBase()->getLibraryPaths();
        doWrite = true;
        break;
      }
      case PING: {
        time(&lastGUIPing);
        sendReplyToGUI(socket, cmd, "OK");
        
        break;
      }
      case SELECT_MATERIAL_IN_HOST: {
        RESET_IPC_BUFFER

        QString objectName, materialName;
        commandStream >> objectName >> materialName;
        // Send the command via the command stack
        RealityBase* rb = RealityBase::getRealityBase();
        rb->commandStackPush(materialName);
        rb->commandStackPush(objectName);
        // "smha" => select material in host app
        rb->commandStackPush("smha");

        sendReplyToGUI(socket, cmd, "OK");
        break;
      } 

      default:
        break;
    }

    if (doWrite) {
      quint32 dataLenght = dataBuffer.length();
      zmq::message_t reply(dataLenght);
      memcpy((void *) reply.data (), dataBuffer.data(), dataLenght);
      socket.send(reply);
    }

  } //try
  catch( std::exception e ) {
    RE_LOG_WARN() << "Exception in ReIPC: " << e.what();
  }
}


void CommandPollingThread::sceneLoaded() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_SCENE_LOADED;

  transmit(buffer);  
}

void CommandPollingThread::objectAdded( const QString objectName ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_OBJECT_ADDED << objectName;

  transmit(buffer);
}

void CommandPollingThread::objectDeleted( const QString objectName ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_OBJECT_DELETED << objectName;

  transmit(buffer);
}

void CommandPollingThread::objectRenamed( const QString objectID, const QString newName ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_OBJECT_RENAMED << objectID << newName;

  transmit(buffer);
}

void CommandPollingThread::objectIDRenamed( const QString objectID, const QString newID ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_OBJECT_ID_RENAMED << objectID << newID;

  transmit(buffer);
}

void CommandPollingThread::objectVisibilityChanged( const QString objectID, 
                                                    const bool isVisible )
{
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_OBJECT_VISIBILITY_CHANGED << objectID << isVisible;

  transmit(buffer);
}



void CommandPollingThread::lightAdded( const QString lightID ) {
  if (!guiAppIsRunning) {
    return;
  }

  QByteArray buffer;
  QDataStream dataStream(&buffer,QIODevice::WriteOnly);
  ReLightPtr light = RealitySceneData->getLight(lightID);
  dataStream << (quint16) HOST_LIGHT_ADDED << lightID;
  light->serialize(dataStream);
  transmit(buffer);
}

void CommandPollingThread::lightDeleted( const QString lightID ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_LIGHT_DELETED << lightID;

  transmit(buffer);
}

void CommandPollingThread::allLightsRemoved() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_LIGHTS_REMOVE_ALL;

  transmit(buffer);
}


void CommandPollingThread::lightRenamed( const QString& lightID, const QString& newName  ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_LIGHT_RENAMED << lightID << newName;

  transmit(buffer);
}


void CommandPollingThread::lightTypeChanged( const QString lightID ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_LIGHT_TYPE_CHANGED << lightID;

  transmit(buffer);
}

void CommandPollingThread::lightUpdated( const QString lightID ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  ReLightPtr light = RealitySceneData->getLight(lightID);
  stream << (quint16) HOST_LIGHT_UPDATED << lightID << light->getName();
  light->serialize(stream);

  transmit(buffer);
}

void CommandPollingThread::GUIPauseMatPreview() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) GUI_PAUSE_MAT_PREVIEW;

  transmit(buffer);
}

void CommandPollingThread::GUIResumeMatPreview() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) GUI_RESUME_MAT_PREVIEW;

  transmit(buffer);
}

void CommandPollingThread::bringGUIToForeGround() {
  if (!guiAppIsRunning) {
    return;
  }

  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) GUI_BRING_TO_FRONT;
  transmit(buffer);
}

//! Notifies the GUI that a new camera has been added to the scene
void CommandPollingThread::cameraAdded( const QString cameraID ) {
  if (!guiAppIsRunning) {
    return;
  }

  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_CAMERA_ADDED << cameraID;
  transmit(buffer);
}

void CommandPollingThread::cameraRenamed( const QString objectID, const QString newName ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_CAMERA_RENAMED << objectID << newName;
  transmit(buffer);
}

void CommandPollingThread::cameraRemoved( const QString objectID ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_CAMERA_DELETED << objectID;
  transmit(buffer);
}

void CommandPollingThread::hostCameraSelected( const QString camID ) {
  if (!guiAppIsRunning) {
    return;
  }
  // RE_LOG_INFO() << "Host switched camera to " << camID;
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_CAMERA_SELECTED << camID;
  transmit(buffer);
}


void CommandPollingThread::cameraDataChanged( const QString cameraID ) {
  if (!guiAppIsRunning) {
    return;
  }
  ReCameraPtr cam = RealitySceneData->getCamera(cameraID);
  if (cam.isNull()) {
    return;
  }

  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_CAMERA_CHANGED << cameraID;
  cam->serialize(stream);

  transmit(buffer);
}

void CommandPollingThread::refreshCameraList() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) REFRESH_CAMERA_LIST;

  transmit(buffer);
}

void CommandPollingThread::exportStarted( const int numObjects ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) EXP_EXPORT_STARTED << numObjects;
  transmit(buffer);
};

void CommandPollingThread::exportingObject( const QString objectName ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) EXP_EXPORTING_OBJECT << objectName;
  transmit(buffer);
};

void CommandPollingThread::exportFinished() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) EXP_EXPORT_FINISHED;
  transmit(buffer);
};

void CommandPollingThread::renderDimensionsNotSet() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) RENDER_DIMENSIONS_NOT_SET;
  transmit(buffer);
}

void CommandPollingThread::frameSizeChanged( const uint width, const uint height ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_FRAME_SIZE_UPDATED << (quint32) width << (quint32) height;

  transmit(buffer);      
}

void CommandPollingThread::setNewScene() {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) SET_NEW_SCENE;

  transmit(buffer);
}

void CommandPollingThread::materialUpdated( const QString matName, const QString objectName ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_MATERIAL_UPDATED << matName << objectName;

  transmit(buffer);
}

void CommandPollingThread::materialTypeChanged( const QString objectID, 
                                                const QString materialName,
                                                const ReMaterialType newType ) 
{
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_MATERIAL_TYPE_CHANGED 
         << objectID << materialName << (quint16) newType;

  transmit(buffer);
}

void CommandPollingThread::updateMatFromClipboardData( const QString& objectID, 
                                                       const QString& materialID, 
                                                       const QString& clipboardData,
                                                       const ReQtMaterialImporter::ReplaceTextureMode texMode ) {
  QJson::Parser parser;
  bool ok;

  QVariantMap matData = parser.parse(clipboardData.toUtf8(), &ok).toMap();
  if (!ok) {
    return;
  }
  ReMaterialType matType = ReMaterial::typeFromName(matData.value("type").toString());

  ReMaterialPtr targetMat = RealitySceneData->getMaterial(objectID, materialID);

  if (targetMat.isNull()) {
    return;
  }

  ReQtMaterialImporterPtr matImporter = ReQtMaterialImporterFactory::getImporter(matType);
  matImporter->importFromClipboard(targetMat, matData, texMode);
}

void CommandPollingThread::changeMaterialType( const QString& objectID, 
                                               const QString& materialID,
                                               const ReMaterialType newType )
{
  auto obj = RealitySceneData->getObject(objectID);
  auto oldType = obj->getMaterial(materialID)->getType();
  if (newType != oldType) {
    obj->changeMaterialType(materialID, newType);
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_MATERIAL_TYPE_CHANGED 
         << objectID << materialID << (quint16) newType;

  transmit(buffer);
}


void CommandPollingThread::applyAcselShader( const QString& objectID,
                                             const QString& materialID,
                                             const QString& setID ) {
  QString shaderCode;
  auto acsel = ReAcsel::getInstance();
  ReMaterialType matType;
  if (acsel->findShader(setID, materialID, shaderCode, matType)) {
    // Find out if the material types match. If not we need to first
    // convert the material to the new type
    auto obj = RealitySceneData->getObject(objectID);
    auto oldType = obj->getMaterial(materialID)->getType();
    if (matType != oldType) {
      obj->changeMaterialType(materialID, matType);
    }
    // Get a pointer to the new material
    auto matPtr = obj->getMaterial(materialID);
    // convert the JSON Data to a QVariantMap
    QJson::Parser parser;
    bool ok;

    QVariantMap shaderData = parser.parse(shaderCode.toUtf8(), &ok).toMap();
    //! This should never happen. Just in case...
    if (!ok) {
      return;
    }

    // Then apply the shader settings
    auto matImporter = ReQtMaterialImporterFactory::getImporter(matType);
    matImporter->importFromClipboard(matPtr, shaderData, ReQtMaterialImporter::Keep);
    if (!guiAppIsRunning) {
      return;
    }
    QByteArray buffer;
    QDataStream stream(&buffer,QIODevice::WriteOnly);
    stream << (quint16) HOST_MATERIAL_TYPE_CHANGED 
           << objectID << materialID << (quint16) matType;

    transmit(buffer);
  }  
}

void CommandPollingThread::applyUniversalShader( const QString& objectID, 
                                                 const QString& materialID,
                                                 const QString& shaderID ) {
  QVariantMap shaderInfo;
  auto acsel = ReAcsel::getInstance();
  if (acsel->findUniversalShader(shaderID, shaderInfo)) {
    auto obj = RealitySceneData->getObject(objectID);
    auto matPtr = obj->getMaterial(materialID);
    // convert the JSON Data to a QVariantMap
    QJson::Parser parser;
    bool ok;

    QVariantMap shaderData = parser.parse(
                               shaderInfo["ShaderCode"].toString().toUtf8(), 
                               &ok
                             ).toMap();
    //! This should never happen. Just in case...
    if (!ok) {
      RE_LOG_INFO() << "Universal shader definition is incorrect: " 
                    << shaderID;
      return;
    }

    // Find out if the material types match. If not, then we need to first
    // convert the material to the new type
    auto oldType = obj->getMaterial(materialID)->getType();
    ReMaterialType newType = static_cast<ReMaterialType>(
                               ReMaterial::typeFromName(shaderData["type"].toString())
                             );
    if (newType == MatUndefined) {
      RE_LOG_WARN() << QString(
                         "Universal shader %1 for material %2::%3 is"
                         " of type undefined"
                       )
                       .arg(shaderID)
                       .arg(objectID)
                       .arg(materialID);
      return;
    }
    if (newType != oldType) {
      obj->changeMaterialType(materialID, newType);
      // The old material has been deleted and a new instance has
      // been created so we need to update the pointer to the material
      // or we are going to update an invalid area of memory
      matPtr = obj->getMaterial(materialID);
    }

    // Then apply the shader settings
    auto matImporter = ReQtMaterialImporterFactory::getImporter(newType);
    matImporter->importFromClipboard(matPtr, shaderData, ReQtMaterialImporter::Replace);
    if (!guiAppIsRunning) {
      return;
    }
    QByteArray buffer;
    QDataStream stream(&buffer,QIODevice::WriteOnly);
    stream << (quint16) HOST_MATERIAL_TYPE_CHANGED 
           << objectID << materialID << (quint16) newType;

    transmit(buffer);
  }  
}

void CommandPollingThread::revertToOriginalShader( const QString& objectID, 
                                                   const QString& materialID ) {
  auto obj = RealitySceneData->getObject( objectID );
  if (obj.isNull()) {
    RE_LOG_WARN() << "Invalid object name";
    return;
  }
  obj->revertToOriginalShader(materialID);
  // Notify the client that the material has changed and needs to be
  // reloaded
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_MATERIAL_TYPE_CHANGED 
         << objectID << materialID 
         << (quint16) obj->getMaterial(materialID)->getType();

  transmit(buffer);
}

void CommandPollingThread::synchronizeChannelTexture( 
       const QString& objectID, 
       const QString& sourceMaterialName,
       const QString& channelName,
       const QString& targetMaterialName 
     ) 
{
  auto obj = RealitySceneData->getObject(objectID);
  if (obj.isNull()) {
    return;
  }
  auto baseMat = obj->getMaterial(sourceMaterialName);
  if (baseMat.isNull()) {
    return;
  }
  auto targetMat = obj->getMaterial(targetMaterialName);
  if (targetMat.isNull()) {
    return;
  }
  // We preserve the name of the texture in the target material
  QString textureName;
  auto ch = targetMat->getChannel(channelName);
  if (!ch.isNull()) {
    textureName = ch->getName();
  }
  else {
    textureName = QString("%1.tex").arg(channelName);
  }

  ReTexturePtr t = baseMat->getNamedValue(channelName).value<ReTexturePtr>();
  if (!t.isNull()) {
    QVariant vt;
    vt.setValue<ReTexturePtr>(
      ReTexturePtr(
        TextureCreator::createTexture(textureName, t)
      )
    );
    auto t2 = vt.value<ReTexturePtr>();
    targetMat->addTextureToCatalog(t2);
    t2->reparent(targetMat.data());
    targetMat->setNamedValue(channelName, vt);
  }
}

void CommandPollingThread::materialSelected( const QString& objectName, const QString& matID ) {
  if (!guiAppIsRunning) {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) HOST_MATERIAL_SELECTED << objectName << matID;

  transmit(buffer);
}

void CommandPollingThread::closeGUI() {
  // Send the message to the GUI to close.
  if (guiAppIsRunning) {
#   if IPC_DEBUG
      RE_LOG_DEBUG() << QString(
        "IPC shutDown() calling the GUI with signal %1"
      ).arg(CLOSE_REALITY);
#   endif
    QByteArray buffer;
    QDataStream stream(&buffer,QIODevice::WriteOnly);
    stream << (quint16) CLOSE_REALITY;
    transmit(buffer);
  }  
}

void CommandPollingThread::shutDown() {
  QMutexLocker locker(&threadRunningFlagLock);
  isActive = false;
}
    

void CommandPollingThread::sendReplyToGUI( zmq::socket_t& socket, 
                                           const IPC_MESSAGES cmd, 
                                           const int value) 
{
  dataBuffer.clear();
  dataStream.device()->seek(0);
  dataStream << (quint16) cmd << value;

  quint32 dataLenght = dataBuffer.length();
  zmq::message_t reply(dataLenght);
  memcpy((void *) reply.data (), dataBuffer.data(), dataLenght);
  socket.send(reply);
}


bool CommandPollingThread::makeNewSubTexture( const QString& objectID, 
                                              const QString& materialName,
                                              const QString& masterTextureName,
                                              const QString& newTextureName,
                                              const QString& channelName,
                                              const ReTextureType textureType,
                                              const ReTexture::ReTextureDataType dataType ) const
{
  ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
  if (!obj.isNull()) {
    ReMaterialPtr mat = obj->getMaterial(materialName);
    if (!mat.isNull()) {
      ReTexturePtr mainTex = mat->getTexture(masterTextureName);
      if (!mainTex.isNull()) {
        ReTexturePtr newTex = ReTexturePtr(
          TextureCreator::createTexture(
            newTextureName, 
            textureType,
            mat.data(),
            dataType
          ) 
        );
        mat->addTextureToCatalog(newTex);
        ReComplexTexturePtr ctex = mainTex.dynamicCast<ReComplexTexture>();
        if (!ctex.isNull()) {
          ctex->setChannel(channelName, newTex);
        }
        else {
          // Try to set the texture using the named value
          QVariant v;
          v.setValue<ReTexturePtr>(newTex);
          mainTex->setNamedValue("texture", v);
        }
        return true;
      }
    }
  }
  return false;
}

bool CommandPollingThread::linkSubTexture( const QString& objectID, 
                                           const QString& materialName,
                                           const QString& channelName,
                                           const QString& textureName,
                                           const QString& subTextureName ) 
{
  ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
  if (!obj.isNull()) {
    ReMaterialPtr mat = obj->getMaterial(materialName);
    if (!mat.isNull()) {
      ReComplexTexturePtr mTex = mat->getTexture(textureName)
                                   .dynamicCast<ReComplexTexture>();
      if (!mTex.isNull()) {
        mTex->setChannel(channelName, mat->getTexture(subTextureName));
        return true;
      }
    }
  }
  return false;
}

bool CommandPollingThread::unlinkSubTexture( const QString& objectID, 
                                             const QString& materialName,
                                             const QString& textureName,
                                             const QString& channelName )
{
  ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
  if (!obj.isNull()) {
    ReMaterialPtr mat = obj->getMaterial(materialName);
    if (!mat.isNull()) {
      ReComplexTexturePtr mTex = mat->getTexture(textureName)
                                   .dynamicCast<ReComplexTexture>();
      if (!mTex.isNull()) {
        mTex->deleteChannelTexture(channelName);
        return true;
      }
    }
  }
  return false;  
}

void CommandPollingThread::replaceTexture( const QString& objectID, 
                                           const QString& materialName,
                                           const QString& channelName,
                                           const QString& jsonTextureData,
                                           const QString& masterTextureName )
{
  auto obj = RealitySceneData->getObject(objectID);
  if (obj.isNull()) {
    RE_LOG_WARN() << "Error: object " << objectID << " does not exist!";
    return;
  }

  auto mat = obj->getMaterial(materialName);
  if (mat.isNull()) {
    RE_LOG_WARN() << "Error: material " << materialName << " does not exist!";
    return;
  }
  ReTexturePtr masterTexture;
  if (!masterTextureName.isEmpty()) {
    masterTexture = mat->getTexture(masterTextureName);
  }
  mat->replaceTexture(channelName, jsonTextureData, masterTexture);
}

void CommandPollingThread::uiMaterialEdited( const QString& objectID, 
                                             const QString& materialName,
                                             const QString& propertyName,
                                             const QVariant& value,
                                             const QString& textureName ) const
{
  ReGeometryObjectPtr obj = RealitySceneData->getObject(objectID);
  if (!obj.isNull()) {
    ReMaterialPtr mat = obj->getMaterial(materialName);
    if (!mat.isNull()) {
      mat->setEdited();
      if (textureName != "") {
        ReTexturePtr tex = mat->getTexture(textureName);
        if (!tex.isNull()) {
          tex->setNamedValue(propertyName, value);                
        }
      }
      else {
        mat->setNamedValue(propertyName,value);
      }
    }
  }
}

void CommandPollingThread::updateAnimationLimits(const int startFrame, 
                                                 const int endFrame, 
                                                 const int fps)
{
  if (!guiAppIsRunning) {
    return;
  }

  QByteArray buffer;
  QDataStream stream(&buffer,QIODevice::WriteOnly);
  stream << (quint16) UPDATE_ANIMATION_LIMITS 
         << startFrame << endFrame << fps;
  transmit(buffer);  
}


} // namespace