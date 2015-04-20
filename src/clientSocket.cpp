#include <iostream>
#include <clientSocket.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

ClientSocket::ClientSocket(const std::string *address, char *port, const addrinfo &hints,
                           const int poolSize):
  Socket(address, port, hints, poolSize)
{
  // Vazio, apenas constrói a classe base
}

ClientSocket::ClientSocket(int socketDescriptor, char *port, int poolSize):
  Socket(socketDescriptor, port, poolSize)
{

}

int ClientSocket::send(const std::string buffer, int flags)
{
  int rc = ::send(this->socketDescriptor, buffer.c_str(), buffer.size(), flags);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro no envio de dados: ")
                             + strerror(errno));
  return rc;
}

int ClientSocket::sendAll(const char *buffer, int length, int flags)
{
  int total = 0;
  int remaining = length;
  int rc = 0;

  while (remaining)
  {
    rc = ::send(this->socketDescriptor, buffer + total, remaining, flags);

    if (rc == -1)
    {
      close(this->socketDescriptor);
      throw std::runtime_error(std::string("Erro em send all: ")
                               + strerror(errno));
    }

    remaining -= rc;
    total += rc;
  }

  return 0;
}

void ClientSocket::connect()
{
  addrinfo *r = this->info;

  if (::connect(this->socketDescriptor, r->ai_addr, r->ai_addrlen) != 0)
  {
    close(this->socketDescriptor);
    throw std::runtime_error(std::string("Erro em Connect: ")
                             + strerror(errno));
  }
}
