#include <iostream>
#include <socketWrapper.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

SocketWrapper::SocketWrapper(const std::string *address, int poolSize,
                             char *port, int maxDescriptors)
{
  // Definindo o socket para responder ao padrão IPV4 ou IPV6, e utilizar TCP.
  memset(&this->hints, 0, sizeof this->hints);
  this->hints.ai_family = AF_UNSPEC;
  this->hints.ai_socktype = SOCK_STREAM;

  if(port)
    this->port = port;
  else
    throw std::runtime_error(std::string("Porta Nula!"));

  int rc = 0;

  if (address)
    rc = getaddrinfo(address->c_str(), this->port, &this->hints, &this->info);
  else
    rc = getaddrinfo(NULL, this->port, &this->hints, &this->info);

  if (rc != 0)
    throw std::runtime_error(std::string("Erro em getaddrinfo: ")
                             + gai_strerror(rc));

  addrinfo *r = this->info;

  socketDescriptor = socket(r->ai_family, r->ai_socktype, r->ai_protocol);

  if (this->socketDescriptor == -1)
    throw std::runtime_error(std::string("Erro na criacao do descritor: ")
                             + strerror(errno));

  if (poolSize > 0)
  {
    try
    {
      this->pool = new char[poolSize];
    }
    catch(std::exception e)
    {
      std::cout << "Erro na alocacao de memoria do pool!";
      throw e;
    }
    this->poolSize = poolSize;
  }

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

    for (int i = 0; i < maxDescriptors; i++)
      this->descriptors[i].fd = -1;

    this->maxDescriptors = maxDescriptors;
  }
}

SocketWrapper::SocketWrapper(const std::string *address, const addrinfo &hints,
                             int poolSize, char *port, int maxDescriptors)
{
  this->hints = hints;

  if(port)
    this->port = port;
  else
    throw std::runtime_error(std::string("Porta Nula!"));

  int rc = 0;

  if (address)
    rc = getaddrinfo(address->c_str(), this->port, &this->hints, &this->info);
  else
    rc = getaddrinfo(NULL, this->port, &this->hints, &this->info);

  if (rc != 0)
    throw std::runtime_error(std::string("Erro em getaddrinfo: ")
                         + gai_strerror(rc));

  addrinfo *r = this->info;

  socketDescriptor = socket(r->ai_family, r->ai_socktype, r->ai_protocol);

  if (this->socketDescriptor == -1)
    throw std::runtime_error(std::string("Erro na criacao do descritor: ")
                           + strerror(errno));

  if (poolSize != 0)
  {
    try
    {
      this->pool = new char[poolSize];
    }
    catch(std::exception e)
    {
        std::cout << "Erro na alocacao de memoria do pool!";
        throw e;
    }
    this->poolSize = poolSize;
  }

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

    for (int i = 0; i < maxDescriptors; i++)
      this->descriptors[i].fd = -1;

    this->maxDescriptors = maxDescriptors;
  }
}

int SocketWrapper::readLine(char *buffer, int length)
{
  if (!this->pool)
  {
    throw std::runtime_error(std::string("Erro: pool para leitura não alocado!"));
  }

  bool done = false;

  // Le poolSize bytes e guarda-os em pool
  if (!this->hasData)
  {
    end = this->receive(this->pool, this->poolSize, 0);

    // end == 0 significa que a conexão foi encerrada
    if(end == 0)
      return 0;

    this->hasData = true;
  }

  int i = 0;

  /*
   * Este laco itera sobre os dados no pool e para uma vez que encontra um
   * separador de linha. Sao lidos no maximo length - 1 bytes do pool.
   */

  while (!done)
  {
    for (unsigned int j = begin; j <= end; ++j, ++i)
    {
      if (j == end)
      {
        if(pool[end] == '\n')
        {
          done = true;
          this->hasData = false;
          this->begin = 0;

          break;
        }

        end = this->receive(this->pool, this->poolSize, 0);
        begin = 0;

        if (end == 0)
          done = true;

        break;
      }

      if (i == length - 2)
      {
        begin = j;
        done = true;

        break;
      }

      buffer[i] = this->pool[j];

      if (pool[j] == '\n')
      {
        begin = j + 1;
        done = true;

        break;
      }
    }
  }

  return i;
}

int SocketWrapper::readAll(char *buffer, int length)
{
  if (!this->pool)
  {
    throw std::runtime_error(std::string("Erro: pool para leitura não alocado!"));
  }

  bool done = false;

  // Le poolSize bytes e guarda-os em pool
  if (!this->hasData)
  {
    end = this->receive(this->pool, this->poolSize, 0);

    if(end == 0)
      return 0;

    this->hasData = true;
  }

  int i = 0;

  /*!
   * Este laco itera sobre os dados no pool e executa até que o pool seja
   * esvaziado ou que o buffer seja totalmente preenchido.
   */

  while (!done)
  {
    for (unsigned int j = begin; j <= end; ++j, ++i)
    {
      if (i == length - 1)
      {
        begin = j;
        done = true;
        break;
      }

      if (j == end)
      {
        end = this->receive(this->pool, this->poolSize, 0);
        begin = 0;

        if (end == 0)
          done = true;

        break;
      }

      buffer[i] = this->pool[j];
    }
  }

  return i;
}

void SocketWrapper::connect()
{
  addrinfo *r = this->info;

  if (::connect(this->socketDescriptor, r->ai_addr, r->ai_addrlen) != 0)
  {
    close(this->socketDescriptor);
    throw std::runtime_error(std::string("Erro em Connect: ")
                             + strerror(errno));
  }

}

int SocketWrapper::send(const std::string buffer, int flags)
{
  int rc = ::send(this->socketDescriptor, buffer.c_str(), buffer.size(), flags);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro no envio de dados: ")
                             + strerror(errno));
  return rc;
}

int SocketWrapper::receive(char *buffer, int length, int flags)
{
  int rc = ::recv(this->socketDescriptor, buffer, length, flags);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro no recebimento de dados: ")
                           + strerror(errno));
  return rc;
}

int SocketWrapper::bind(bool reuseAddress)
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

int SocketWrapper::listen(const int backlog)
{
  int rc = ::listen(this->socketDescriptor, backlog);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro em listen: ") + strerror(errno));

  return rc;
}

SocketWrapper* SocketWrapper::accept(const int poolSize)
{
  socklen_t endpointSize = sizeof(this->endpoint);

  int newDescriptor = ::accept(this->socketDescriptor,
                              (struct sockaddr *)&endpoint, &endpointSize);

  if(newDescriptor == -1)
    throw std::runtime_error(std::string("Erro em accept: ") + strerror(errno));

  return new SocketWrapper(newDescriptor, this->port, poolSize);
}

int SocketWrapper::add(SocketWrapper *socket, int events)
{
  if(socket == NULL)
    throw std::invalid_argument("Erro: socket não pode ser nulo em add!");

  if(this->currentDescriptors >= this->maxDescriptors)
    throw std::length_error("Erro: numero maximo de descritores alcancado.");

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

int SocketWrapper::remove(SocketWrapper *socket)
{
  if(socket == NULL)
    throw std::invalid_argument("Erro: socket não pode ser nulo em remove!");

  if(this->currentDescriptors <= 0)
    throw std::length_error("Erro: lista de descritores vazia.");

  for (int i = 0; i < this->maxDescriptors; i++)
  {
    if (socket->socketDescriptor == this->descriptors[i].fd)
      this->descriptors[i].fd = -1;
  }

  this->currentDescriptors--;

  return 0;
}

int SocketWrapper::poll(int timeout)
{
  int rc = ::poll(this->descriptors, this->maxDescriptors, timeout);

  if(rc == -1)
    throw std::runtime_error(std::string("Erro em poll: ") + strerror(errno));

  return rc;
}

bool SocketWrapper::canRead(SocketWrapper *socket)
{
  if(socket == NULL)
    throw std::invalid_argument("Erro: socket não pode ser nulo em canRead!");

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

bool SocketWrapper::canSend(SocketWrapper *socket)
{
  if(socket == NULL)
    throw std::invalid_argument("Erro: socket não pode ser nulo em canRead!");

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

SocketWrapper::SocketWrapper(int socketDescriptor, char *port, int poolSize)
{
  this->socketDescriptor = socketDescriptor;
  this->port = port;

  try
  {
    this->pool = new char[poolSize];
  }
  catch(std::exception e)
  {
    std::cout << "Erro na alocacao de memoria do pool!";
    throw e;
  }

  this->poolSize = poolSize;
}

SocketWrapper::~SocketWrapper()
{

  if(this->pool)
    delete[] pool;

  if(this->maxDescriptors)
    delete[] descriptors;

  if(info)
    freeaddrinfo(this->info);

  ::close(this->socketDescriptor);
}
