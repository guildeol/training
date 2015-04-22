#include <iostream>
#include <clientSocket.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

/*!
 * \brief Construtor da classe.
 * \param[in] address endereco do servidor desejado.
 * \param[in] hints struct com parametros para criacao do socket.
 * \param[in] port Numero da porta na qual a socket deve se comunicar
 * \param[in] poolSize tamanho do pool que deve ser reservado para leitura.
 * \throw runtime_error ao chamar socket.
 * \throw runtime_error ao chamar getaddrinfo.
 * \throw bad_alloc caso a alocação de pool falhar.
 */
ClientSocket::ClientSocket(const std::string *address, const std::string port,
                           const addrinfo &hints, const int poolSize):
  Socket(address, port, hints, poolSize)
{
  // Vazio, repassa para a classe base
}

/*!
 * \brief Construtor privado da classe, é utilizada pelo servidor em accept.
 *        Existe devido à necessidade de se criar um socket com descritor
 *        especifico.
 * \param[in] socketDescriptor Descritor obtido via accept().
 * \param[in] port Numero da porta na qual a socket deve se comunicar
 * \param[in] poolSize tamanho do pool que deve ser reservado para leitura.
 * \throw bad_alloc caso a alocação de pool falhar.
 */
ClientSocket::ClientSocket(int socketDescriptor, const std::string port,
                           int poolSize):
  Socket(socketDescriptor, port, poolSize)
{
  // Vazio, apenas constrói a classe base
}

/*!
 * \brief Metodo sobrecarregado para enviar uma string via socket
 * \param[in] buffer String a ser enviada.
 * \param[in] flags Parametros opcionais para a transmissao.
 * \throw runtime_error caso chamar ::send.
 */
int ClientSocket::send(const std::string buffer, int flags)
{
  int rc = ::send(this->socketDescriptor, buffer.c_str(), buffer.size(), flags);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro no envio de dados: ")
                             + strerror(errno));
  return rc;
}

/*!
 * \brief Metodo tentar enviar a quantidade de bytes total passada.
 * \param[in] buffer Dados a serem enviadas.
 * \param[in] length Quantidade a ser enviada.
 * \param[in] flags Parametros opcionais para a transmissao.
 * \throw runtime_error caso chamar ::send.
 */
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

/*!
 * \brief Connecta-se ao socket com os parâmetros especificados no construtor.
 * \throw runtime_error caso ::connect falhe.
 */
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
