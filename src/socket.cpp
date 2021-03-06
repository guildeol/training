#include <iostream>
#include <socket.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

Socket::Socket(const std::string *address, const std::string port,
                             const addrinfo &hints, int poolSize)
{
  this->hints = hints;

  this->port.assign(port);

  int rc = 0;

  if (address)
    rc = getaddrinfo(address->c_str(), this->port.c_str(), &this->hints,
                     &this->info);
  else
    rc = getaddrinfo(NULL, this->port.c_str(), &this->hints, &this->info);

  if (rc != 0)
    throw std::runtime_error(std::string("Erro em getaddrinfo: ")
                         + gai_strerror(rc));

  addrinfo *r = this->info;

  this->socketDescriptor = socket(r->ai_family, r->ai_socktype, r->ai_protocol);

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
}

Socket::Socket(int socketDescriptor, std::string port, int poolSize)
{
  this->socketDescriptor = socketDescriptor;
  this->port.assign(port);

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

int Socket::readLine(char *buffer, int length)
{
  int available = length;
  char *found = NULL;
  int copyAmount = 0;

  if (!this->pool)
    throw std::runtime_error(std::string("Erro: pool para leitura não alocado!"));

  while (true)
  {
    // Le poolSize bytes e guarda-os em pool
    if (!this->hasData)
    {
      end = this->receive(this->pool, this->poolSize, 0);

      // end == 0 significa que a conexão foi encerrada
      if(end == 0)
        return 0;

      this->start = 0;
      this->hasData = true;
    }

    found = strchr(this->pool + this->start, '\n');

    if (found)
      copyAmount = found - (this->pool + this->start) + 1;
    else
      copyAmount = this->end - this->start;

    if (copyAmount > available)
      throw std::runtime_error("Erro: tamanho do buffer insuficiente.");

    // Copia todos os dados encontrados, incluindo o ''\n'
    memcpy(buffer, this->pool + this->start, copyAmount);

    if (found)
    {
      this->start += copyAmount;
      this->start %= this->poolSize;
      break;
    }

    available -= copyAmount;
    hasData = false;
  }

  return copyAmount;
}

int Socket::readAll(char *buffer, int length)
{
  int copyAmount = 0;
  int used = 0;

  if (!this->pool)
    throw std::runtime_error(std::string("Erro: pool para leitura não alocado!"));

  while (used < length)
  {
    if (!this->hasData)
    {
      end = this->receive(this->pool, this->poolSize, 0);

      this->start = 0;
      this->hasData = true;

      // end == 0 significa que a conexão foi encerrada
      if(end == 0)
        break;
    }

    copyAmount = (this->end - this->start) % (length + 1);

    if(used + copyAmount > length)
      break;

    memcpy(buffer + used, this->pool + this->start, copyAmount);

    used += copyAmount;

    this->hasData = false;
  }

  return used;
}

int Socket::send(const char *buffer, int length, int flags)
{
  int rc;

  if (length == -1)
    rc = ::send(this->socketDescriptor, buffer, strlen(buffer), flags);
  else
    rc = ::send(this->socketDescriptor, buffer, length, flags);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro no envio de dados: ")
                             + strerror(errno));
  return rc;
}

int Socket::receive(char *buffer, int length, int flags)
{
  int rc = ::recv(this->socketDescriptor, buffer, length, flags);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro no recebimento de dados: ")
                           + strerror(errno));
  return rc;
}

Socket::~Socket()
{

  if(this->pool)
    delete[] pool;

  if(info)
    freeaddrinfo(this->info);

  ::close(this->socketDescriptor);
}
