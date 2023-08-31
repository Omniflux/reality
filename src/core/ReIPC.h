/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

/**
 File: ReIPC.h

 Inter Process Communication classes. The IPC system is meant to connect the hosted plugin
 side of Reality with the UI, which runs as a separate process.

 */

#ifndef RE_IPC_H
#define RE_IPC_H

#include <QMutex>
#include <QThread>
#include <zmq.hpp>

#include "reality_lib_export.h"
#include "ReSharedMemIPC.h"
#include "importers/qt/ReQtMaterialImporter.h"

namespace Reality {
  enum IPC_MESSAGES;
}


#define IPC_DEBUG 0


namespace Reality {

/**
 * This class runs in the background, on the host side, polling messages sent by the 
 * client UI and relying information back and forth.
 */
class REALITY_LIB_EXPORT CommandPollingThread : public QThread {
  
private:
  //! Flag that we use to know when to stop this thread
  bool isActive;

  //! Mutex used to access the <isActive> variable
  QMutex threadRunningFlagLock;

  //! The name of the log file on disk that we use to track the
  //! work of this thread. Useful for debugging
  QString logFileName;

  //! Flag that indicates if the client GUI has opened a connection with the 
  //! data server.
  bool guiAppIsRunning;

  //! The last time the GUI has pinged us
  time_t lastGUIPing;

  QByteArray dataBuffer;
  QDataStream dataStream;

  zmq::context_t ipcContext;
  //! ZMQ socket used to send notifications to the GUI
  zmq::socket_t* notifySocket;

  //! The ID of the application we are communicating with
  HostAppID appID;

  ReSharedMemIPC shmChannel;
  
  //! Handles the transmission of requests to the GUI via the 
  //! ZMQ socket
  void transmit( QByteArray& buffer );

  // Handler for the MAKE_NEW_SUB_TEXTURE message
  bool makeNewSubTexture( const QString& objectID, 
                          const QString& materialName,
                          const QString& masterTextureName,
                          const QString& newTextureName,
                          const QString& channelName,
                          const ReTextureType textureType,
                          const ReTexture::ReTextureDataType dataType ) const;

  bool linkSubTexture( const QString& objectID, 
                       const QString& materialName,
                       const QString& channelName,
                       const QString& masterTextureName,
                       const QString& subTextureName );

  //! Removes a subtexture from a containing texture
  bool unlinkSubTexture( const QString& objectID, 
                         const QString& materialName,
                         const QString& textureName,
                         const QString& channelName );

  //! Replaces a texture with the data from the copy/paste operation
  void replaceTexture( const QString& objectID, 
                       const QString& materialName,
                       const QString& channelName,
                       const QString& jsonTextureData,
                       const QString& masterTextureName );

  void uiMaterialEdited( const QString& objectID, 
                         const QString& materialName,
                         const QString& propertyName,
                         const QVariant& value,
                         const QString& textureName ) const;
  void updateMatFromClipboardData( const QString& objectID, 
                                   const QString& materialID, 
                                   const QString& clipboardData,
                                   const ReQtMaterialImporter::ReplaceTextureMode texMode );

  //! Responds to the message CHANGE_MATERIAL_TYPE and performs the action
  //! of changing the material type.
  void changeMaterialType( const QString& objectID, 
                           const QString& materialID,
                           const ReMaterialType newType );

  //! Responds to the message APPLY_ACSEL_SHADER and performs the action
  //! of changing the material by applying the compatible shader from the
  //! shader set identified by setID
  void applyAcselShader( const QString& objectID, 
                         const QString& materialID,
                         const QString& setID );

  //! Responds to the message APPLY_UNIVERSAL_SHADER and performs the action
  //! of changing the material by applying the universal shader from the
  //! identified by shaderID
  void applyUniversalShader( const QString& objectID, 
                             const QString& materialID,
                             const QString& shaderID );

  //! This method creates a texture from another texture
  //! and it links it to a channel. This is used when synchronizing materials
  //! together. The texture is a duplicate of the texture from the same
  //! channel but from another material.
  //! 
  //! \param objectID The name of the object containing the textures to be
  //!                 synchronized
  //! \param sourceMaterialName The material containing the original texture
  //!                           to be duplicated
  //! \param channelName The channel containing the texture
  //! \param targetMaterialName The name of the material receiving the texture
  void synchronizeChannelTexture( const QString& objectID, 
                                  const QString& sourceMaterialName,
                                  const QString& channelName,
                                  const QString& targetMaterialName );

  //! Revert a material to use the original shader that was stored when the 
  //! material was first created.
  void revertToOriginalShader( const QString& objectID, const QString& materialID );

  /**
   * Communicates with the host-app to set the scene as dirty
   */
  void setSceneAsDirty();

public:

    CommandPollingThread( const HostAppID appID = Poser );

    ~CommandPollingThread();

    /**
     Sends a string to the GUI.
    */
    void sendReplyToGUI( zmq::socket_t& socket, const IPC_MESSAGES cmd, const QString str);

    /**
     Sends an int to the GUI. Overloaded method.
    */
    void sendReplyToGUI(zmq::socket_t& socket, const IPC_MESSAGES cmd, const int value);

    //! Tell the GUI that it's time to quit
    void closeGUI();

    //! Informs the GUI that it needs to pause the updating of the material
    //! preview
    void GUIPauseMatPreview();

    //! Informs the GUI that it needs to resume the updating of the material
    //! preview
    void GUIResumeMatPreview();

    //! Returns the last GUI ping
    inline time_t getLastGUIPing() {
      return lastGUIPing;
    }

    /**
     Processes the requests from the GUI app and delivers the data requested.
    */
    void processRequest(zmq::message_t& request, zmq::socket_t& socket);

    //! Notifies the GUI that a new object has been added to the scene
    void objectAdded( const QString objectName );

    void objectDeleted( const QString objectName );

    void objectRenamed( const QString objectID, const QString newName );

    //! Signal sent when an object changes its ID
    void objectIDRenamed( const QString objectID, const QString newID );

    //! Signal sent when an object visibility changes
    void objectVisibilityChanged( const QString objectID, const bool isVisible );

    //! Notifies the GUI that a new light has been added to the scene and
    //! sends the light through the wire
    void lightAdded( const QString lightID );

    void lightDeleted( const QString lightID );

    //! Notifies the GUI that all lights have been removed from the scene.
    void allLightsRemoved();

    void lightTypeChanged( const QString lightID );

    //! Notifies the GUI that a light has been renamed in the host app.
    void lightRenamed( const QString& lightID, const QString& newName  );

    //! Notifies the GUI that the light has been updated
    void lightUpdated( const QString lightID );

    /**
     * Signal the GUI to come to the foreground
     */
    void bringGUIToForeGround();    

    //! Notifies the GUI that a new camera has been added to the scene
    void cameraAdded( const QString cameraID );

    void cameraRenamed( const QString objectID, const QString newName );

    void cameraRemoved( const QString objectID );

    void cameraDataChanged( const QString cameraID );

    void updateAnimationLimits(const int startFrame, const int endFrame, const int fps);

    //! Signals to the UI that the active camera in the host has changed
    //! \param camID The GUID of the active camera.
    void hostCameraSelected( const QString camID );

    void refreshCameraList();

    void frameSizeChanged( const uint width, const uint height );

    void setNewScene();

    //! Informs the GUI that the host-app has loaded a scene
    void sceneLoaded();

    void materialUpdated( const QString matName, const QString objectName );

    void materialSelected( const QString& objectName, const QString& matID );

    //! Signal that the export has started for <numObjects> objects.
    void exportStarted( const int numObjects );

    //! Signal that the exporter is processing object objectName
    void exportingObject( const QString objectName );

    //! Signals that the export has finished.
    void exportFinished();

    void renderDimensionsNotSet();

    /**
     * When the user asks to change the type of material the request is sent to
     * the host-app using the CHANGE_MATERIAL_TYPE message. The message is carried
     * over until the plugin running inside the host receives it and performs the
     * change based on the full data of the original shader.
     * Once the conversion is completed on the host side the message HOST_MATERIAL_TYPE_CHANGED
     * is send to the GUI via this method.
     */
    void materialTypeChanged( const QString objectID, 
                              const QString materialName,
                              const ReMaterialType newType );


    void shutDown();

    //! The body of the thread. Here we received the messages from the GUI and
    //! dispatch them to the processRequest() method.
    void run();
};

extern REALITY_LIB_EXPORT CommandPollingThread* realityIPC;

} // namespace


#endif