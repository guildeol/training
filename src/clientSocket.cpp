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
