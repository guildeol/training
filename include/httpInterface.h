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
#include <clientSocket.h>

#include <ctime>
#include <string>
#include <map>

#define INVALID_METHOD    -1000
#define INVALID_PROTOCOL  -2000

class HTTPInterface
{
public:

  HTTPInterface(std::string &request);

  int validate();

  int respond(int code, ClientSocket *socket);

  std::string method;
  std::string resource;
  std::string protocol;

private:

  void fetch(std::ifstream &file, int code, int &length);
  std::string timeToString(struct tm &t);

  std::string knownMethods;
  std::string knownProtocols;

  std::map<int, std::string> reason = {{200, "OK"}, };
  std::map<int, std::string> responseFiles{{404, "404.html"}};
};

#endif
