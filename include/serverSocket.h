/*!
* \file serverSocket.h
* \brief Cabecalho para classe com funcionalidades para criacao de servidor
*        utilizando como base Socket.
*
* \date 15/04/2015
* \author Guilherme Costa <glhrmcosta91@gmail.com>
*/

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <socket.h>
#include <clientSocket.h>
#include <poll.h>

#include <string>

/*!
* \brief Classe para criacao de um servidor utilizando sockets.
* \class ServerSocket
*/

class ServerSocket : public Socket
{

public:

  ServerSocket(const std::string *address, std::string port,
               const addrinfo &hints, int poolSize = 0, int maxDescriptors = 0);

  ~ServerSocket();

  int bind(bool reuseAddress = true);

  int listen(const int backlog);

  ClientSocket* accept(const int poolSize = 0);

  int add(Socket *socket, int events);

  int remove(Socket *socket);

  bool canSend(Socket *socket);

  bool canRead(Socket *socket);

  int poll(int timeout = -1);

private:

  struct sockaddr_storage endpoint; /*!< Utilizado em accept().*/

  /*!
  * Array de structs utilizado em poll. Deve ser mantido pelo
  * listerner/servidor.
  */
  struct pollfd *descriptors = NULL;
  int currentDescriptors = 0;
  int maxDescriptors = 0;
};

#endif
