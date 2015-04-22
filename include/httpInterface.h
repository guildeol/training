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
#include <vector>

#define OK              200
#define BAD_REQUEST     400
#define NOT_FOUND       404
#define FORBIDDEN       403
#define NOT_IMPLEMENTED 501
#define NOT_SUPPORTED   505

class HTTPInterface
{
public:

  HTTPInterface(std::string &request);

  int validate(std::string &root);

  int addHeader(char *header);

  int respond(int code, std::string &root, ClientSocket *socket);

  std::string method;
  std::string resource;
  std::string protocol;

private:

  void fetch(std::ifstream &file, int code, int &length, std::string &root);
  std::string timeToString(struct tm &t);

  std::string knownMethods;
  std::string knownProtocols;

  std::vector<std::string> headers;

  std::string responseFolder;

  std::map<int, std::string> reason = {{200, "OK"}, {400, "Bad Request"},
                                       {403, "Forbidden"}, {404, "Not Found"},
                                       {501, "Not Implemented"},
                                       {505, "HTTP Version Not Supported"}};
};

#endif
