/*
 *  MatPreviewThread.h
 *  ReGeomExport
 *
 *  Created by Paolo H Ciccone on 6/26/11.
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 *
 */

#ifndef RE_MATERIAL_PREVIEW_H
#define RE_MATERIAL_PREVIEW_H

#include <QHash>
#include <QImage>
#include <QQueue>
#include <QSharedPointer>
#include <QThread>
#include <zmq.hpp>

#include "zeromqTools.h"

namespace Reality {
  class ReLuxRunner;
  typedef QSharedPointer<ReLuxRunner> ReLuxRunnerPtr;
}


// The MP part means "Material Preview"
//! This constant defines the transport system for the communication from
//! a client and the material preview thread
#define RE_MP_REQUEST_TRANSPORT "inproc://MP_requester"
#define RE_MP_QUIT_REQUEST      "quit"
#define RE_MP_PREVIEW_REQUEST   "matprev"
#define RE_MP_PREVIEW_INIT      "init"

//! Interval, in milliseconds, at which we check if the request queue has
//! new requests. This interval allows for multiple requests to be placed
//! before a preview is processed. This, in turn, allows us to prioritize
//! the requests. For example, we can give the material preview a priority
//! of one and a procedural texture a priority of 0, highest, which will
//! cause the processing from the highest to the lowest, providing accurate
//! results.
const unsigned short RE_MP_CLOCK_INTERVAL = 200;

const quint8 MPM_PROCTEX_SIZE     = 144;
const quint8 MPM_MATPREVIEW_SIZE  = 120;

namespace Reality {

/**
 Simple container to send requets for material previews to the work thread
 */
class PreviewRequest {

public:

  //! The GUID of the material  
  QString materialName;

  //! Unique ID for this preview
  QString previewID;

  //! The name of the scene to use. We can use plane, sphere, face, and eye scenes.
  //! In the scene the %1 placeholder is for the material definition itself. The 
  //! %2 placeholder is for the name of the material.
  QString sceneName;

  //! The Lux material definition for the material that we want to preview
  QString materialDefinition;

  bool isProceduralTexture;

  bool forceRefresh;

  PreviewRequest() {

  }

  PreviewRequest( PreviewRequest& pr2 ) {
    materialName        = pr2.materialName;
    previewID           = pr2.previewID;
    sceneName           = pr2.sceneName;
    materialDefinition  = pr2.materialDefinition;
    isProceduralTexture = pr2.isProceduralTexture;
    forceRefresh        = pr2.forceRefresh;
  }

  PreviewRequest(const QString& mn, 
                 const QString& pi, 
                 const QString& sn,
                 const QString& md,
                 const bool isProceduralTexture = false,
                 const bool forceRefresh = false ) : 
    materialName(mn),
    previewID(pi),
    sceneName(sn), 
    materialDefinition(md),
    isProceduralTexture(isProceduralTexture),
    forceRefresh(forceRefresh)
  {
    // nothing
  }

  void set(const QString& mn, 
           const QString& pi, 
           const QString& sn,
           const QString& md,
           const bool isPT = false,
           const bool fr = false )
  {
    materialName        = mn;
    previewID           = pi;
    sceneName           = sn;
    materialDefinition  = md;
    isProceduralTexture = isPT;
    forceRefresh        = fr;
  }
};

// typedef QSharedPointer<PreviewRequest> PreviewRequestPtr; 
typedef QSharedPointer<QImage> ReImagePtr;

// Forward declaration
class ReMaterialPreview;

/**
 Class that runs the external luxconsole process to obtain the material previews.

 This thread is started by the <ReMaterialPreview> thread and it's stopped by that thread
 as well when the program ends. Communication between the two threads is done via a ZeroMQ
 socket. The format of the data is simple: numeric code followed by pointer to data.

 The numeric code is also used to stop this thread. 
 */
class RePreviewProducer : public QThread {

  Q_OBJECT

private:

  //! We keep the preview scene in this string variable. Often the same scene is used
  //! for material preview. By caching it we avoid opening and loading the same resource
  //! over and over again.
  QString previewSceneTemplate;

  //! To cache the scene used we need to store the name of the last scene and compare it
  //! with the one from the current request.
  QString lastSceneName;

  //! Pointer to a Lux process object used to run the preview
  ReLuxRunnerPtr luxProc;

public:
  RePreviewProducer( ReMaterialPreview* owner );

protected:
  //! Starts a new process and returns true if the process has been
  //! started successfully. False otherwise.
  bool startLuxProcess();

  //! The thread's main body. It's only running a loop until the end
  void run();

protected slots:
  //! Processes the preview request from the material preview thread
  void processPreviewRequest( PreviewRequest* req );

signals:
  // Signal emitted when a material preview has been created and is ready
  // to be sent to the requester.
  void materialPreviewReady( QString matName, 
                             QString previewID, 
                             bool isProceduralTexture,
                             QImage* img );
};


/**
 * Class used to provide a list of material previews. It uses <RePreviewProducer>.
 */
class ReMaterialPreview : public QThread {

  Q_OBJECT

private:
  //! Singleton implementation
  static ReMaterialPreview* instance;

  //! The cache of material previews. 
  QHash<QString, ReImagePtr> previewCache;

  ushort maxCacheSize;

  //! The Producer Thread that generates the actual previews
  RePreviewProducer* previewProducer;

  QQueue<PreviewRequest*> requestQueue;

  //! The ZMQ context object needed for communication
  zmq::context_t ipcContext;

  bool initialized;

public:

  //! ctor
  ReMaterialPreview();
  
  //~ !ctor
  ~ReMaterialPreview();

  /**
   * Return the single instance of this class. The first call to this method
   * creates the instance.
   */
  static ReMaterialPreview* getInstance();

  /**
   * This method can be called statically to send a request for a preview
   * to the Material Preview maker.
   */
  void sendPreviewRequest( const QString& matName, 
                           const QString& previewID,
                           const QString& previewType,
                           const QString& matDefinition,
                           const bool isProceduralTexture = false,
                           const bool forceRefresh = false );
  //! Static method used to send a request to quit to this thread
  void quit();

  inline zmq::context_t* getIPCContext() {
    return &ipcContext;
  }

  inline bool isInitialized() {
    return initialized;
  }
  
public slots:
  
  //! Add a request to the request queue
  void addRequestToQueue( zmqReadDataStream& data, zmq::socket_t& receiver );
  void processPreviewQueue();  
  void run();
  void previewDone(QString materialName, 
                   QString previewID,
                   bool isProceduralTexture,
                   QImage* img);
  void previewFailed(QString materialName);
  void setCacheSize( const ushort newSize );

signals:
  void previewReady(QString materialName, QString previewID, QImage* img);
  void previewAborted(QString);
  void previewInProgress();
  void previewRequested( PreviewRequest* prReq );
};

} // namespace


#endif
