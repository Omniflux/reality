/**
  \file

  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReMaterialPreview.h"
#include "RealityBase.h"
#include <boost/atomic.hpp>

using namespace Reality;

// Variable used to control the exit of the threads. When set to false
// the threads exists. 
boost::atomic<bool> keepRunning;

RePreviewProducer::RePreviewProducer( ReMaterialPreview* owner ) {
  connect(owner, SIGNAL(previewRequested(PreviewRequest*)),
          this, SLOT(processPreviewRequest(PreviewRequest*)) );
}

void RePreviewProducer::processPreviewRequest( PreviewRequest* req ) 
{
  if (req->sceneName != lastSceneName) {
    QFile sceneFile;
    if (req->isProceduralTexture) {
      sceneFile.setFileName(":/textResources/ProceduralNoisePreviewer.lxs");
    }
    else {
      sceneFile.setFileName(QString(":/textResources/%1_preview.lxs")
                            .arg(req->sceneName));
    }
    sceneFile.open(QIODevice::ReadOnly);
    previewSceneTemplate = sceneFile.readAll();
    sceneFile.close();
    lastSceneName = req->sceneName;
  }
  QString previewScene;
  if (!req->isProceduralTexture) {
    previewScene = previewSceneTemplate
                   .arg(req->materialDefinition)
                   .arg(req->materialName);        
  }
  else {
    previewScene = previewSceneTemplate.arg(req->materialDefinition);
  }
  // RE_LOG_INFO() << "== Preview: " << req->materialName.toStdString()
  //               << " " << req->previewID.toStdString();
  // Write to stdin the scene definition
  luxProc->writeToStdin(previewScene);
  // Grab the result as a stream of bytes
  luxProc->waitForFinished();
  QByteArray frameBuffer = luxProc->readAllStandardOutput();

  QImage* preview;
  // Size of the preview
  quint16 pSize = req->isProceduralTexture ? MPM_PROCTEX_SIZE : MPM_MATPREVIEW_SIZE;
  // Create the image, this is 32-bit aligned in format 0xAARRGGBB
  preview = new QImage(pSize,pSize,QImage::Format_RGB32);

  // Copy the pixels from the framebuffer to the image buffer setting up
  // an implicit alpha of 0xff. The framebuffer is formatted to be 0xRRGGBB
  int cursor = 0;          
  for (int i = 0; i < pSize; ++i) {
    uchar* line = preview->scanLine(i);
    for (int p = 0; p < pSize; ++p) {
      QRgb* pixel = (QRgb*)(line+p*4);
      *pixel = qRgb(
        frameBuffer[cursor],frameBuffer[cursor+1],frameBuffer[cursor+2]
      );
      cursor += 3;
    }
  }
  emit materialPreviewReady( 
    req->materialName, req->previewID, req->isProceduralTexture, preview
  );
  delete req;
  startLuxProcess();  
}


bool RePreviewProducer::startLuxProcess() {
  if (!luxProc.isNull() && luxProc->getState() == QProcess::NotRunning) {
    luxProc.clear();
  }
  luxProc = ReLuxRunnerPtr(new ReLuxRunner());
  auto result = luxProc->runLuxConsole("", true, false);
  return (result == ReLuxRunner::LR_NORMAL_EXIT);
}

void RePreviewProducer::run() {
  startLuxProcess();  
  while(keepRunning) {
    msleep(100);
  }
  luxProc->killProcess();
};

// Static members
ReMaterialPreview* ReMaterialPreview::instance = NULL;

//! Singleton implementation
ReMaterialPreview* ReMaterialPreview::getInstance() {
  if (!instance) {
    instance = new ReMaterialPreview();
    instance->start();
  }
  return instance;
}

ReMaterialPreview::ReMaterialPreview() {
  previewProducer = NULL;
  ReConfigurationPtr config = RealityBase::getConfiguration();
  maxCacheSize = config->value(RE_CFG_MAT_PREVIEW_CACHE_SIZE).toInt(); 
};

ReMaterialPreview::~ReMaterialPreview() {
}

// Static method used to send the request 
void ReMaterialPreview::sendPreviewRequest( const QString& matName, 
                                            const QString& previewID,
                                            const QString& previewType,
                                            const QString& matDefinition,
                                            const bool isProceduralTexture,
                                            const bool forceRefresh )
{
  // ZMQ can fail with EINTR or other signals. In that case
  // it will throw an exception, which will crash Reality because
  // Qt does not handle exceptions. The try/catch block takes care
  // of avoiding the crash but, if an exception occours, we end up
  // with no preview. The while loop takes care of resending the message
  // in case of ZMQ exception.
  while(true) {
    try {
      auto matPreview = ReMaterialPreview::getInstance();
      zmq::context_t* cont = matPreview->getIPCContext();
      zmq::socket_t sender(*cont, ZMQ_PAIR);
      sender.connect(RE_MP_REQUEST_TRANSPORT);

      zmqWriteDataStream dataStream(sender);
      if (!matPreview->isInitialized()) {
        dataStream << QString(RE_MP_PREVIEW_INIT);
      }
      dataStream.send();

      dataStream << QString(RE_MP_PREVIEW_REQUEST)
                 << matName
                 << previewID
                 << previewType
                 << matDefinition
                 << isProceduralTexture
                 << forceRefresh;

      dataStream.send();
      break;
    }
    catch( zmq::error_t e ) {
      RE_LOG_WARN() << "ZMQ exception: " << e.what();
    }
  }
}

// Static method used to quit the thread 
void ReMaterialPreview::quit()
{
  keepRunning = false;
}

void ReMaterialPreview::addRequestToQueue( zmqReadDataStream& data, 
                                           zmq::socket_t& receiver ) 
{
  QString matName;
  QString previewID;
  QString previewType;
  QString matDefinition;
  bool isProceduralTexture;
  bool forceRefresh;

  data >> matName >> previewID >> previewType
       >> matDefinition >> isProceduralTexture >> forceRefresh;

  requestQueue.enqueue(new PreviewRequest(
    matName, previewID, previewType,
    matDefinition, isProceduralTexture, forceRefresh
  ));
};

void ReMaterialPreview::processPreviewQueue() 
{
  while( requestQueue.count() > 0 ) {
    auto req = requestQueue.dequeue();
    bool isProceduralTexture = req->isProceduralTexture;
    // Check if we already have the preview in the cache
    if (!req->forceRefresh && 
         previewCache.contains(req->materialName)) {

      // The receiver of this pointer takes ownership and is responsible
      // for freeing this resource. We use copy() to avoid a shallow copy of
      // the image, which would cause a crash when we free it from the
      // queue
      QImage* preview = new QImage(
        previewCache.value(req->materialName)->copy()
      );
      emit previewReady(req->materialName, 
                        req->previewID,
                        preview);

      delete req;
      return;
    }

    // If we reached the maximum amount of objects in the cache then pop 
    // the oldest one. Procedural textures are not cached
    if ( !isProceduralTexture && (previewCache.count() >= maxCacheSize) ) {
      while( previewCache.count() >= maxCacheSize ) { 
        previewCache.erase(previewCache.begin());
      }
    }
    // The target object processing the requests takes ownership of the 
    // pointer and is responsible for deleting it
    emit previewRequested( req );
  }
}

void ReMaterialPreview::run() {
  keepRunning = true;
  initialized = false;
  // Configure the socket for the inboud requests from the client
  zmq::socket_t receiver(ipcContext, ZMQ_PAIR);
  receiver.bind(RE_MP_REQUEST_TRANSPORT);
  zmqSetNoLinger(receiver);

  previewProducer = new RePreviewProducer(this);
  // Connect to the producer so that we receive notifications when
  // a preview is ready
  connect(
    previewProducer, 
    SIGNAL(materialPreviewReady(QString, QString, bool,QImage*)), 
    this, 
    SLOT(previewDone(QString, QString, bool, QImage*))
  );

  previewProducer->start();
  // We use a clock to check the request queue every RE_MP_CLOCK_INTERVAL 
  // milliseconds
  QTime clock;
  clock.start();
  while(keepRunning) {
    /*
     * Communication with the client/handling the requests of new
     * material previews
     */     
    try {
      if ( zmqHasMessages(receiver) ) {
        zmqReadDataStream requestStream(receiver);

        QString command;
        requestStream >> command;
        if (command == RE_MP_PREVIEW_REQUEST) {
          // read the data and queue the request
          addRequestToQueue(requestStream, receiver);
        }
        else if ( command == RE_MP_PREVIEW_INIT ) {
          initialized = true;
        }
      }
    }
    catch( zmq::error_t e ) {
      RE_LOG_WARN() << "ZMQ exception: " << e.what();
    }

    if (clock.elapsed() >= RE_MP_CLOCK_INTERVAL) {
      if (requestQueue.count() > 0) {
        processPreviewQueue();
      }
      clock.restart();
    }
  }
  previewProducer->wait();
  delete previewProducer;
}

void ReMaterialPreview::previewDone(QString materialName, 
                                    QString previewID,
                                    bool isProceduralTexture,
                                    QImage* img) 
{
  // Procedural textures are not cached
  if (!isProceduralTexture) {
    // Add the bitmap to the cache 
    previewCache[materialName] = ReImagePtr(img);

    // The receiver of this pointer takes ownership and is responsible
    // for freeing this resource. We use copy() to avoid a shallow copy of
    // the image, which would cause a crash when we free it from the
    // queue
    QImage* preview = new QImage(img->copy());
    emit previewReady(materialName, previewID, preview);
  }
  else {
    emit previewReady(materialName, previewID, img);
  }
}

void ReMaterialPreview::previewFailed(QString materialName) {
  // Clean up the queue
  emit previewAborted(materialName);    
}

void ReMaterialPreview::setCacheSize( const ushort newSize ) {
  maxCacheSize = newSize;
}
