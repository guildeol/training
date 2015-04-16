/*!
* \file httpInterface.h
* \brief Cabecalho para classe com funcionalidades para interpretação e execucao
         seguindo protocol HTTP 1.1.
*
* \date 15/04/2015
* \author Guilherme Costa <glhrmcosta91@gmail.com>
*/

#ifndef HTTPINTERFACE_H
#define HTTPINTERFACE_H

#include <socket.h>

#include <string>

class HTTPInterface
{
public:

  HTTPInterface();
  ~HTTPInterface();

  int parseRequest(std::string request);

  int respond(int code, Socket &socket);

  std::string method;
  std::string resource;
  std::string protocol;
};

#endif
