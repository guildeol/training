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
* \brief Classe para criacao de um client utilizando sockets.
* \class clientSocket
*/

class ClientSocket : public Socket
{

public:

  ClientSocket(const std::string *address, char *port, const addrinfo &hints,
               const int poolSize = 0);

  /*Tenta conexao ao endereço especificado no construtor*/
  void connect();

  /*Tenta enviar todos os dados em buffer atraves do socket*/
  int sendAll(char *buffer, int length);

};

#endif
