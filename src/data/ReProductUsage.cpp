/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReProductUsage.h"
#include "ReHttpClient.h"
#include "RealityBase.h"

using namespace Reality;
using boost::asio::ip::tcp;

void ReProductUsage::run() {
  timer = new boost::asio::deadline_timer(ioservice,
                                          boost::posix_time::seconds(timeInterval));    
  timer->async_wait(boost::bind(&ReProductUsage::sendUsageData, this));

  while( true ) {
    {
      // Check if the thread has been requested to stop
      QMutexLocker locker(&flagLock);
      if (!keepRunning) {
        break;
      }
    }
    ioservice.run();
  }
}

ReProductUsage* ReProductUsage::getInstance() {
  static ReProductUsage* instance = NULL;

  if (instance == NULL) {
    instance = new ReProductUsage();
  }

  return instance;
}

void ReProductUsage::storeFileName( const QString& fileName ) {
  QMutexLocker locker(&dataLock);
  fileNames << fileName;
}

void ReProductUsage::storeFileNames( const QStringList& newFileNames ) {
  QMutexLocker locker(&dataLock);
  for (int i = 0; i < newFileNames.count(); i++) {
    if (newFileNames[i].startsWith('#')) {
      continue;
    }
    // Add the file name after normalizing the path to a Runtime and after
    // removing the leading "@" added by normalizeRuntimePath()
//    fileNames << "/" + normalizeRuntimePath(newFileNames[i]).mid(1);
    QString newPath = normalizeRuntimePath(newFileNames[i]);
    if (newPath[0] == '@') {
      newPath[0] = '/';
    }
    fileNames << newPath;
  }
}

void ReProductUsage::sendPayload() {
  static QString idToken;
  if (fileNames.count() == 0) {
    return;
  }

  auto userData = ReUserData::getUserData();
  if (idToken.isEmpty()) {
    idToken = sha1(userData[decryptString(RE_USER_DATA_SERIAL_NO)]);
  }

  QMap<QString,QVariant> postData;
  // We are going to clear the list only if the transmission is successfull.
  // We use the sentFiles collection to keep track of what is actually sent.
  // If we were to clear the fileNames list we would potentially incour into
  // the case where the fileNames list is updated between the transmission
  // and the clearing operation, with the result of removing file names that
  // had not yet been transmitted.
  QStringList sentFiles;
  QString postDataStr;
  {
    QMutexLocker locker(&dataLock);
    sentFiles = fileNames.toList();
    postData["files"] = QVariant(sentFiles);
    postData["id"] = idToken;
    auto hostID = RealityBase::getRealityBase()->getHostAppID();
    switch( hostID ) {
      case Poser: {
         postData["host"] = "ps";
         break;
      }
      case DAZStudio: {
         postData["host"] = "ds";
         break;
      }
      default: {
         postData["host"] = "na";
      }
    }                         
    postData["hostVer"] = RealityBase::getRealityBase()->getHostVersion();

    QJson::Serializer json;
    postDataStr = json.serialize(postData);
  }
  #ifndef NDEBUG
    RE_LOG_INFO() << "Sending stat data";  
    RE_LOG_INFO() << "Payload: " << postDataStr;
  #endif

  // Delete the file names from the list only if the transmission is successful.
  // Otherwise it will try again later.
  if ( ReHttpClient::sendHttpRequest("stats.preta3d.com", 
                                     "/productUsage.php", 
                                     postDataStr) ) {
    QMutexLocker locker(&dataLock);
    for (int i = 0; i < sentFiles.count(); i++) {
      fileNames.remove(sentFiles[i]);
    }
  };
}

void ReProductUsage::sendUsageData() {
  if (!keepRunning) {
    return;
  }
  // Respect the choice of the users and send the data only if they agree to 
  // join the survey.
  if (RealityBase::getConfiguration()->value(RE_CFG_JOIN_PRODUCT_USAGE_SURVEY).toBool()) {
    sendPayload();
  }
  // Reschedule the timer for timeInterval second in the future:
  timer->expires_from_now(boost::posix_time::seconds(timeInterval));
  timer->async_wait(boost::bind(&ReProductUsage::sendUsageData, this));    
}

void ReProductUsage::stop() {
  {
    QMutexLocker lock(&flagLock);
    keepRunning = false;
  }
  timer->cancel();
  thread->join();
  // Send the last batch, if present.
  sendPayload();
}

