/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REALITY_DATA_RELAY_H
#define REALITY_DATA_RELAY_H

#include <QMutexLocker>
#include <QThread>
#include <zmq.hpp>

#include "ReIPC.h"
#include "ReSceneData.h"


namespace Reality {  

/**
 * This class implements a thread that communicates from the GUI to the data server and brokers
 * the data to and from the server.  
 */
class RealityDataRelay : public QThread {

Q_OBJECT

private:  
  HostAppID appID;
  bool isActive;
  QMutex lock;

  zmq::context_t ipcContext;
  zmq::socket_t* serverCommandInterface;

  QString serverAddress;
  int ipcPort,
      ipcPubSubPort;
  
  //! The IPC channel that uses shared memory
  ReSharedMemIPC shmChannel;

  //! Used to create the socket used to communicate with the data server
  //! The method sets also the options necessary.
  void createDataServerSendingSocket() {
    // Create the connection to talk with the data server, the host-side
    // plugin.
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
    QString ipAddress = QString(IPC_CLIENT_ADDRESS).arg(serverAddress).arg(ipcPort);
    serverCommandInterface = new zmq::socket_t(ipcContext, ZMQ_REQ);
    int hwm = 3000;
    serverCommandInterface->setsockopt(ZMQ_SNDHWM, &hwm, sizeof(int));

    serverCommandInterface->connect(ipAddress.toAscii().data());

    // Set the option for the socket to avoid lingering when closing the program
    // This makes it possible to shutdown the GUI even if the server (Poser/Studio)
    // is not running anymore.
    int linger = 0;
    serverCommandInterface->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));    
  };

  //! This method calls the GUI and forwards one of the messages emitted by POSER.
  //! Specifically it forwards the HOST_OBJECT_ADDED, HOST_OBJECT_DELETE and HOST_OBJECT_RENAMED
  //! messages
  void forwardNotification( zmq::message_t& update );

public:

  RealityDataRelay( const QString _serverAddress = "localhost", 
                    const HostAppID appID = Poser ) : 
    appID(appID),
    ipcContext(1)
  {
    serverAddress = _serverAddress;
  };

  ~RealityDataRelay() {
    delete serverCommandInterface;
  }

  void stop();

  //! Reimplemented from the base class
  void run();

  //! Sends a message to the data server and reads the reply. This reply is
  //! then send back to the caller of this method.
  void sendMessageToServer( const IPC_MESSAGES msg, const QVariantMap* args = 0);

  bool isReady() {
    QMutexLocker locker(&lock);
    return isActive;
  }

signals:

  //! Emitted when Reality needs to be moved to the foreground
  void bringUIToFront();

  //! Emitted when the client-side IPC services are up and running 
  void upAndRunning();

  //! Emitted when the user adds an object to the Host's scene
  void objectAdded(const QString objName);
  //! Emitted when the user renames an object in the Host's scene
  void objectRenamed(const QString oldName, const QString newName);
  //! Emitted when the host app changes the ID of an object
  void objectIDRenamed(const QString oldID, const QString newID);
  //! Emitted when the visibility of an object changes
  void objectVisibilityChanged(const QString objectID, const bool isVisible);

  //! Emitted when the user delete an object from the Host's scene
  void objectDeleted(const QString objName);

  //! Emitted when the requested object has been deserialized and received 
  void objectReady( ReGeometryObject* obj );

  //! Emitted when the requested set of objects is ready
  void objectsReady( ReGeometryObjectDictionary objs );

  //! Emitted when the requested list of object names is ready
  void objectNamesReady( QStringList objNames );

  //! Emitted when the requested number of objects in the scene has been received
  void numObjectsReady( int numObjects );

  void materialUpdated( QString materialName, QString objectID );

  //! Emitted when the host-app has finished changing the material type, as requested 
  //! by the user.
  void materialTypeChanged( const QString objectID,  const QString materialName, const quint16 newType);

  //!   Emitted when a material is selected in the Host App. 
  void materialSelected( const QString& objectID, const QString& matID );

  // Volumes
  void volumesTransferred();

  // Cameras

  //! Emitted when the list of cameras has been transferred
  void camerasReady();
  //! Emitted when the user renames a camera in the Host's scene
  void cameraRenamed(const QString oldName, const QString newName);
  void cameraChanged(const QString cameraID);
  void cameraAdded(const QString cameraID);
  void cameraRemoved(const QString cameraID);
  void hostCameraSelected(const QString cameraID);

  // Lights

  //! Emitted when the requested list of lights has been transferred
  void lightsReady();

  // // Emitted when a GUI-requested light has been received from the host-app
  // void lightReady(const QString lightID);

  //! Emitted when the GUI receives a notification from the host-app that a
  //! light has been added to the scene. This signal will trigger the request,
  //! from the GUI, of transferring the light from the host-app.
  void lightAdded(const QString lightID);

  //! Emitted when the GUI receives a notification from the host-app that a 
  //! light has been deleted from the scene. 
  void lightDeleted(const QString lightID);

  //! Emitted when all lights have been removed
  void allLightsRemoved();

  //! Emitted when the GUI receives a notification from the host-app that a 
  //! light has been renamed.
  void lightRenamed(const QString lightID, const QString newName);

  void lightTypeChanged(const QString lightID);

  //! Emitted when the host-app side has sent an update to the light
  void lightUpdated(const QString lightID);

  //! Emitted when the host app ID has been received
  void hostAppIDReady();

  //! Emitted when a requested material has been received
  void materialReady(QString objectID, QString materialName);

  //! Emitted when the user resets the scene in the host app
  void sceneHasBeenReset();

  //! Emitted when new output data has been loaded from the hot-app side
  void outputDataReady();

  //!  Emitted when the user changes the frame size in the host-app
  void frameSizeChanged();

  //! Emitted when the export to the renderer started
  void exportStarted( int numObjects);

  //! Emitted when an object is currently exported to the renderer. This is
  //! used to alert the GUI of the process so that it can inform the user
  void exportingObject( const QString objName );

  //! Emitted when the host-app side has finished exporting the scene to the renderer.
  void exportFinished();

  //! Emitted to alert the user that the render dimensions are not set
  void renderDimensionsNotSet();

  //! Emitted when a scene has been loaded in the host-app
  void sceneLoaded();
  
  //! Emitted when it's time to close the GUI app.
  void close();

  //! Emitted when the GUI needs to pause or resume the material preview
  //! If the bool in the parameter is true then the material preview
  //! must be paused, otherwise it is resumed.
  void pauseMaterialPreview(bool);
};

//! IPC Thread that communicates with the server side of Reality and exchanges data
//! and events.
extern Reality::RealityDataRelay* realityDataRelay;

}

#endif
