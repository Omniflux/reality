/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_HTTP_CLIENT_H
#define RE_HTTP_CLIENT_H

#include "ReDefs.h"
#include <QtCore>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Reality {

/**
 * Simple Http client used to send requests to our website
 */
class RE_LIB_ACCESS ReHttpClient {

public:
  //! Constructor: ReHttpClient
  ReHttpClient() {
  };

  //! Destructor: ReHttpClient
 ~ReHttpClient() {
  };

  //! Calls a web page and send POST data formatted in a JSON string
  static bool sendHttpRequest( QString server, QString file, QString data );
  
};

} // namespace

#endif
