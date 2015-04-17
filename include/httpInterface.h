/*!
* \file httpInterface.h
* \brief Cabecalho para classe com funcionalidades para interpretação e execucao
         seguindo protocolo HTTP 1.1.
*
* \date 15/04/2015
* \author Guilherme Costa <glhrmcosta91@gmail.com>
*/

#ifndef HTTPINTERFACE_H
#define HTTPINTERFACE_H

#include <socket.h>

#include <string>
#include <vector>

#define INVALID_METHOD    -1000
#define INVALID_PROTOCOL  -2000

class HTTPInterface
{
public:

  HTTPInterface(std::string &request);

  int validate();

  int respond(int code, Socket &socket);

  std::string method;
  std::string resource;
  std::string protocol;

private:

  const std::string knownMethods;
  const std::string knownProtocols;
};

#endif
