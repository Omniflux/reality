/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_PRODUCT_USAGE_H
#define RE_PRODUCT_USAGE_H

#include <QtCore>
#include <QStringList>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include "ReLogger.h"
#include "ReUserData.h"
#include "ReTools.h"
#include "qjson/src/serializer.h"

namespace Reality {

/**
 * A class to gather information about products used in Poser and DAZ Studio and 
 * designed to send such data to the Pret-a-3D server for data analysis.
 */
class ReProductUsage {

private:
  //! The list of file names used by the customer in setting up her scenes
  QSet<QString> fileNames;
  //! Flag that communicates to the thread if it needs to keep running.
  bool keepRunning;

  //! Seconds of interval between timer events
  static const quint16 timeInterval = 60;

  //! Mutexes used to access the fileNames array and the keepRunning
  //! flag.
  QMutex dataLock,
         flagLock;

  //! The Thread object
  boost::thread* thread;
  //! Timer for the posting of the usage data to the server
  boost::asio::deadline_timer* timer;
  boost::asio::io_service ioservice;

  //! Thread function
  void run() ;

  //! Constructor: ReProductUsage. Hidden for singleton implementation
  ReProductUsage() : keepRunning(true) {
    thread = new boost::thread(&ReProductUsage::run, this);
  };

public:

  //! Singleton access method
  static ReProductUsage* getInstance();

  //! Destructor: ReProductUsage
 ~ReProductUsage() {
    delete timer;
    delete thread;
  };

  //! Stores a file name into the collection to be sent to the server.
  void storeFileName( const QString& fileName );

  //! Stores a list of file names into the collection to be sent to the server.
  void storeFileNames( const QStringList& newFileNames );

  //! Send the current batch of names to the Pret-a-3D server. The list is emptied
  //! once sent.
  void sendPayload();

  //! Timer handler. Initiates the transfer if the conditions are right, and then 
  //! restarts the timer.
  void sendUsageData();

  //! Stops the thread and sends the last batch of data, if present.
  void stop();

};

} // namespace

#endif
