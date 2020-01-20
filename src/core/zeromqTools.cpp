/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#include "zeromqTools.h"
#include "relogger.h"

QString zmqReceiveString(zmq::socket_t& socket) {
  try {
    zmq::message_t message;
    socket.recv(&message);
    return QString(QByteArray(static_cast<char*>(message.data()),message.size()));
  }
  catch( zmq::error_t e ) {
    RE_LOG_INFO() << "Exception in zmqReceiveString() "
                  << e.what();
  }
  return "";
}

bool zmqSendString(zmq::socket_t& socket, const QString string) {
  try {
    int length = string.size();
    zmq::message_t message(length);
    memcpy (message.data(), string.toAscii(), length);
    return (socket.send(message));
  }
  catch( zmq::error_t e ) {
    RE_LOG_INFO() << "Exception in zmqSendString() "
                  << e.what();
  }  
  return false;
}

void* zmqReceivePointer(zmq::socket_t& socket) {
  try {
    zmq::message_t message;
    socket.recv(&message);
    int msgSize = message.size();
    void* ptr = NULL;
    if (msgSize == sizeof(void*)) {
      memcpy(&ptr, message.data(), msgSize);
    }
    else {
      RE_LOG_WARN() << "Discrepancy in size of message and size of pointer";
    }
    return ptr;
  }
  catch( zmq::error_t e ) {
    RE_LOG_INFO() << "Exception in zmqReceivePointer() "
                  << e.what();
  }
  return NULL;
}

bool zmqSendPointer(zmq::socket_t& socket, const void* ptr) {  
  try {
    int length = sizeof(ptr);
    zmq::message_t message(length);
    memcpy(message.data(), &ptr, length);

    return (socket.send(message));
  }
  catch( zmq::error_t e ) {
    RE_LOG_INFO() << "Exception in zmqSendPointer() "
                  << e.what();
  }
  return false;
}

//! Check is there are pending messages for a given socket. This operation
//! is non-blocking so it can be used to poll a socket. 
//! Returns true if there are pending messages, false otherwise
bool zmqHasMessages( zmq::socket_t& zmqSocket, unsigned int timeOut ) {
  try {
    zmq::pollitem_t zmqItem = { zmqSocket, 0, ZMQ_POLLIN, 0 };
    zmq::poll(&zmqItem, 1, timeOut);
    return (zmqItem.revents & ZMQ_POLLIN);
  }
  catch( zmq::error_t e ) {
    RE_LOG_INFO() << "Exception in zmqHasMessages: " << e.num() << " - " << e.what();
  }
  return false;
}

void zmqSetNoLinger( zmq::socket_t& socket ) {
  int linger = 0;
  socket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
}

void zmqSetNoLinger( zmq::socket_t* socket ) {
  zmqSetNoLinger(*socket);
}
