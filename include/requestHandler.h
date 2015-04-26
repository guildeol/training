/*!
* \file httpInterface.h
* \brief Cabecalho para classe com funcionalidades para interpretação e execucao
         seguindo protocolo HTTP 1.1.
*
* \date 15/04/2015
* \author Guilherme Costa <glhrmcosta91@gmail.com>
*/

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <socket.h>
#include <clientSocket.h>
#include <tokenBucket.h>

#include <fstream>
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

/*!
 * \class RequestHandler
 * \brief Classe para interpretacao e resolução de requisicoes HTTP.
 */
class RequestHandler
{
public:

  RequestHandler(std::string &request);

  int validate(std::string &root);

  void addHeader(char *header);

  bool respond(int code, std::string &root, ClientSocket *socket);

  std::string method;
  std::string resource;
  std::string protocol;

private:

  void fetch(int code, std::string &root);
  std::string timeToString(struct tm &t);

  void sendHeaders(ClientSocket *socket, int code, int fileLength);

  std::string knownMethods;
  std::string knownProtocols;

  std::vector<std::string> headers;

  std::string responseFolder;

  std::map<int, std::string> reason = {{200, "OK"}, {400, "Bad Request"},
                                       {403, "Forbidden"}, {404, "Not Found"},
                                       {501, "Not Implemented"},
                                       {505, "HTTP Version Not Supported"}};

  TokenBucket bucket;
  int totalSent = 0;
  std::ifstream file;
  int fileLength = 0;
};

#endif
