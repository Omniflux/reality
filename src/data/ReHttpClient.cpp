/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReHttpClient.h"
#include "ReLogger.h"

using namespace Reality;
using boost::asio::ip::tcp;

bool ReHttpClient::sendHttpRequest( QString server, QString path, QString data ) {
  try  {
    boost::asio::io_service ioservice;
    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(ioservice);
    tcp::resolver::query query(server.toStdString(), "http");
    tcp::resolver::iterator endpointIterator = resolver.resolve(query);

    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket(ioservice);
    boost::asio::connect(socket, endpointIterator);

    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    boost::asio::streambuf request;
    std::ostream requestStream(&request);
    requestStream << "POST " << path.toStdString() << " HTTP/1.0\r\n";
    requestStream << "Host: " << server.toStdString() << "\r\n";
    requestStream << "Accept: */*\r\n";
    requestStream << "Content-Type: application/json\r\n";
    requestStream << "Content-Length: " << data.length() << "\r\n";
    requestStream << "Connection: close\r\n\r\n";
    requestStream << data.toStdString();

    // Send the request.
    boost::asio::write(socket, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    // Check that response is OK.
    std::istream responseStream(&response);
    std::string http_version;
    responseStream >> http_version;
    unsigned int status_code;
    responseStream >> status_code;
    std::string status_message;
    std::getline(responseStream, status_message);
    if (!responseStream || http_version.substr(0, 5) != "HTTP/")
    {
      RE_LOG_INFO() << "Invalid response\n";
      return false;
    }
    if (status_code != 200) {
      RE_LOG_INFO() << "Response returned with status code " << status_code << "\n";
      return false;
    }

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    while (std::getline(responseStream, header) && header != "\r") {
      RE_LOG_INFO() << header << "\n";
    }
    RE_LOG_INFO() << "\n";

    // Write whatever content we already have to output.
    if (response.size() > 0)
      RE_LOG_INFO() << &response;

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(socket, response,
           boost::asio::transfer_at_least(1), error)) {
      RE_LOG_INFO() << &response;
    }
    // if (error != boost::asio::error::eof)
    //   throw boost::system::system_error(error);
  }
  catch (std::exception& e)  {
    RE_LOG_INFO() << "Exception: " << e.what() << "\n";
  }

  return true;
}
