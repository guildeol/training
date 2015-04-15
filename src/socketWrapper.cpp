#include <iostream>
#include <socketWrapper.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

SocketWrapper::SocketWrapper(const std::string *address, char *port,
                             const addrinfo &hints, int poolSize)
{
  this->hints = hints;

  if(!port)
    throw std::runtime_error(std::string("Porta Nula!"));

  this->port = port;

  int rc = 0;

  if (address)
    rc = getaddrinfo(address->c_str(), this->port, &this->hints, &this->info);
  else
    rc = getaddrinfo(NULL, this->port, &this->hints, &this->info);

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

SocketWrapper::~SocketWrapper()
{

  if(this->pool)
    delete[] pool;

  if(info)
    freeaddrinfo(this->info);

  ::close(this->socketDescriptor);
}
