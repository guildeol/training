/*!
* \file serverSocket.h
* \brief Cabecalho para classe com funcionalidades para criacao de client
*        utilizando como base Socket.
*
* \date 17/04/2015
* \author Guilherme Costa <glhrmcosta91@gmail.com>
*/

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <socket.h>
#include <poll.h>

#include <string>

/*!
* \brief Classe para criacao de um cliente utilizando sockets.
* \class clientSocket
*/

class ServerSocket;

class ClientSocket : public Socket
{

friend class ServerSocket;

public:

  ClientSocket(const std::string *address, const std::string port,
               const addrinfo &hints, const int poolSize = 0);

  /*Tenta conexao ao endereço especificado no construtor*/
  void connect();

  /* Sobrecarga para enviar um objeto string via socket */
  int send(const std::string buffer, int flags = 0);

  /* Tenta enviar todos os dados em buffer atraves do socket*/
  int sendAll(const char *buffer, int length, int flags = 0);

private:

  /* Construtor privado, utilizado em accept.*/
  ClientSocket(int socketDescriptor, const std::string port, int poolSize);
};

#endif
