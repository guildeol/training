#include <iostream>
#include <serverSocket.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

ServerSocket::ServerSocket(const std::string *address, char *port,
                           const addrinfo &hints, int poolSize,
                           int maxDescriptors):
  Socket(address, port, hints, poolSize)
{
  if (maxDescriptors > 0)
  {
    try
    {
      this->descriptors = new struct pollfd[maxDescriptors];
    }
    catch(std::exception e)
    {
      std::cout << "Erro na alocacao de memoria do vetor de descritores!";
      throw e;
    }

    //Inicializando os descritores do array
    for (int i = 0; i < maxDescriptors; i++)
      this->descriptors[i].fd = -1;

    this->maxDescriptors = maxDescriptors;
  }
}

ServerSocket::~ServerSocket()
{
  if(this->descriptors)
    delete[] this->descriptors;
}

int ServerSocket::bind(bool reuseAddress)
{
  //Setando opcao para reutilizar o endereço
  if (reuseAddress)
  {
    int yes = 1;
    setsockopt(this->socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &yes,
    sizeof(int));
  }

  int rc = ::bind(this->socketDescriptor, this->info->ai_addr,
                  this->info->ai_addrlen);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro em bind: ") + strerror(errno));

  return rc;
}

int ServerSocket::listen(const int backlog)
{
  int rc = ::listen(this->socketDescriptor, backlog);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro em listen: ") + strerror(errno));

  return rc;
}

ClientSocket* ServerSocket::accept(const int poolSize)
{
  socklen_t endpointSize = sizeof(this->endpoint);

  int newDescriptor = ::accept(this->socketDescriptor,
                              (struct sockaddr *)&endpoint, &endpointSize);

  if(newDescriptor == -1)
    throw std::runtime_error(std::string("Erro em accept: ") + strerror(errno));

  ClientSocket *newSocket = NULL;

  try
  {
    newSocket = new ClientSocket(newDescriptor, this->port, poolSize);
  }
  catch (std::exception e)
  {
    std::cout << "Erro na alocacao de memoria do novo Socket apos accept!";
    throw e;
  }

  return newSocket;
}

int ServerSocket::add(Socket *socket, int events)
{
  if(socket == NULL)
  throw std::invalid_argument(std::string("Erro: parametro socket nulo!"));

  if(this->currentDescriptors >= this->maxDescriptors)
    throw std::length_error(std::string("Erro: numero maximo de descritores alcancado."));

  for (int i = 0; i < this->maxDescriptors; i++)
  {
    if (this->descriptors[i].fd == -1)
    {
      this->descriptors[i].fd = socket->socketDescriptor;
      this->descriptors[i].events = events;
      this->descriptors[i].revents = 0;
      break;
    }
  }

  this->currentDescriptors++;

  return 0;
}

int ServerSocket::remove(Socket *socket)
{
  if(socket == NULL)
    throw std::invalid_argument(std::string("Erro: parametro socket nulo!"));

  if(this->currentDescriptors <= 0)
    throw std::length_error(std::string("Erro: lista de descritores vazia."));

  for (int i = 0; i < this->maxDescriptors; i++)
  {
    if (socket->socketDescriptor == this->descriptors[i].fd)
    this->descriptors[i].fd = -1;
  }

  this->currentDescriptors--;

  return 0;
}

int ServerSocket::poll(int timeout)
{
  int rc = ::poll(this->descriptors, this->maxDescriptors, timeout);

  if(rc == -1)
    throw std::runtime_error(std::string("Erro em poll: ") + strerror(errno));

  return rc;
}

bool ServerSocket::canRead(Socket *socket)
{
  if(socket == NULL)
    throw std::invalid_argument(std::string("Erro: parametro socket nulo!"));

  for (int i = 0; i < this->maxDescriptors; i++)
  {
    if (socket->socketDescriptor == this->descriptors[i].fd)
    {
      if (this->descriptors[i].revents & POLLIN)
        return true;

      break;
    }
  }

  return false;
}

bool ServerSocket::canSend(Socket *socket)
{
  if(socket == NULL)
    throw std::invalid_argument(std::string("Erro: parametro socket nulo!"));

  for (int i = 0; i < this->maxDescriptors; i++)
  {
    if (socket->socketDescriptor == this->descriptors[i].fd)
    {
      if (this->descriptors[i].revents & POLLOUT)
        return true;

      break;
    }
  }

  return false;
}
