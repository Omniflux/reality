/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#ifndef ZEROMQ_TOOLS_H
#define ZEROMQ_TOOLS_H

#include "ReDefs.h"
#include "zmq.hpp"
#include <QtCore>
#include "ReLogger.h"
#include "reality_lib_export.h"

/***********************************************
 * ZMQ utility functions
 ***********************************************/

//! Sends a QString via a ZeroMQ socket
REALITY_LIB_EXPORT bool zmqSendString(zmq::socket_t& socket, const QString string);

//! Retrieves a C string form the socket and returns is as a QString.
//! This function blocks if there are no strings ready
REALITY_LIB_EXPORT QString zmqReceiveString(zmq::socket_t& socket);

//! Receives a pointer from the socket.
REALITY_LIB_EXPORT void* zmqReceivePointer(zmq::socket_t& socket);
//! Sends a pointer via a socket
REALITY_LIB_EXPORT bool zmqSendPointer(zmq::socket_t& socket, const void* ptr);

//! Check is there are pending messages for a given socket. This operation
//! is non-blocking so it can be used to poll a socket. 
//! Returns true if there are pending messages, false otherwise
REALITY_LIB_EXPORT bool zmqHasMessages( zmq::socket_t& zmqSocket, unsigned int timeOut = 100 );
REALITY_LIB_EXPORT inline bool zmqHasMessages( zmq::socket_t* zmqSocket,
                                          unsigned int timeOut = 100 ) 
{
  return zmqHasMessages(*zmqSocket, timeOut);
}

REALITY_LIB_EXPORT void zmqSetNoLinger( zmq::socket_t& socket );
REALITY_LIB_EXPORT void zmqSetNoLinger( zmq::socket_t* socket );

// REALITY_LIB_EXPORT void zmqSetDataStream( QDataStream& dataStream, zmq::message_t& msg );

/**
  Simple class to read the content of a ZMQ message using the same facilities
  provided by QDataStream.

  Usage:
   zmqReadDataStream inStream(someZMQSocket);
   outStream >> data1 >> data2 >> data3 >> ... >> dataN;
 */
class zmqReadDataStream : public QDataStream {
private:
  QByteArray rawData;

  zmq::socket_t& theSocket;
public:
  zmqReadDataStream( zmq::socket_t& s ) :   
    QDataStream( &rawData, QIODevice::ReadOnly ),
    theSocket(s)
  {
    zmq::message_t msg;
    theSocket.recv(&msg);
    quint32 dataLength = msg.size();
    rawData.resize(dataLength+1);
    memcpy(rawData.data(), (void *) msg.data(), dataLength);
  }
};

/**
  Simple output class to stream the content of a ZMQ message using the same 
  facilities provided by QDataStream.

  Usage:
   zmqWriteDataStream outStream(someZMQSocket);
   outStream << data1 << data2 << data3 << ... << dataN;
   outStream.send(); 
 */
class zmqWriteDataStream : public QDataStream {
private:
  QByteArray rawData;

  zmq::socket_t& theSocket;

public:

  zmqWriteDataStream( zmq::socket_t& s) : 
    QDataStream( &rawData, QIODevice::WriteOnly ),
    theSocket(s)
  {
  };

  void reset() {
    rawData.clear();
    device()->seek(0);
  }

  void send() {
    quint32 dataLength = rawData.length();
    zmq::message_t reply(dataLength);
    memcpy((void *) reply.data(), rawData.data(), dataLength);
    theSocket.send(reply);
    reset();
  }

};


#endif
