/*!
 * \file socketWrapper.h
 * \brief Cabecalho para Wrapper da biblioteca C de Sockets do Unix.
 *
 * \date 06/04/2015
 * \author Guilherme Costa <glhrmcosta91@gmail.com>
 * TODO: Adicionar excecoes proprias!
 */

#ifndef SOCKETWRAPPER_H
#define SOCKETWRAPPER_H

/* types.h Utilizado por compatibilidade (ver notas em man socket).*/
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

/* Para addrinfo.*/
#include <netdb.h>
/* Para close.*/
#include <unistd.h>

/* Para memset.*/
#include <cstring>

#include <string>

/*!
 * \class SocketWrapper
 * \brief Wrapper para as funcoes da biblioteca de sockets em C.
 */
class SocketWrapper
{
  /*!
   * Construtor com parametro adicional para
   * especificacao do comportamento do socket.
   * \param[in] address endereco do servidor desejado.
   * \param[in] hints struct com parametros para criacao do socket.
   * \param[in] port Numero da porta na qual a socket deve se comunicar
   * \param[in] poolSize tamanho do pool que deve ser reservado para leitura.
   * \throw runtime_error ao chamar socket.
   * \throw runtime_error ao chamar getaddrinfo.
   * \throw bad_alloc caso a alocação de pool falhar.
   */
  SocketWrapper(const std::string *address, char *port = "80",
                addrinfo *hints = NULL, int poolSize = 0);

  /*Destrutor. Libera recursos alocados.*/
  ~SocketWrapper();

  /*Tenta conexao ao endereço especificado no construtor*/
  void connect();

  /*!
  * \brief Envia uma mensagem através do socket criado.
  * \param[in] buffer Mensagem a ser enviada
  * \param[in] flags Flags de controle repassadas ao socket.
  * \return Quantidade de bytes enviados
  * \throw runtime_error ao chamar send.
  */
  int send(const std::string buffer, int flags = 0);

  /*!
  * \brief Recebe uma mensagem através do socket criado.
  * \param[out] buffer Variavel para guardar os dados lidos
  * \param[in] length Quantidade de bytes a serem lidos
  * \param[in] flags Flags de controle repassadas ao socket.
  * \return Quantidade de bytes lidos
  * \throw runtime_error ao chamar recv.
  */
  int receive(char *buffer, int length, int flags = 0);

  /*!
  * \brief Lê uma linha recebida através do socket, até um máximo de length - 1.
  * \param[out] buffer Variavel para guardar os dados lidos.
  * \param[in] length Tamanho maximo do buffer.
  * \return Quantidade de bytes lidos
  */
  int readLine(char *buffer, int length);

  /*!
  * \brief Lê todos os dados enviados através do socket, até a conexão ser
  *        encerrada.
  * \param[out] buffer Variavel para guardar os dados lidos.
  * \param[in] length Tamanho do buffer para armazenamento.
  * \return Quantidade de bytes lidos
  */
  int readAll(char *buffer, int length);

private:

  int socketDescriptor; /*!< Descritor do socket.*/
  struct addrinfo *hints = NULL; /*!< Utilizada para configuração do socket.*/
  struct addrinfo *info = NULL; /*!< Guarda resultado do getaddrinfo().*/

  struct sockaddr_storage endpoint; /*!< Utilizado em accept().*/

  char *port; /*!< Porta com a qual o socket se comunica.*/

  /* Ambas as funcoes readLine e readAll operam as variaveis abaixo.*/
  char *pool = NULL; /*!< Pool para leitura através de buffer.*/
  unsigned int poolSize = 0; /*!< Tamanho maximo do pool.*/
  unsigned int begin = 0; /*!< Variavel para controle de leitura de pool*/
  unsigned int end = 0; /*!< Variavel para controle de leitura de pool*/
  bool hasData = false; /*!< Variavel para indicar se ha dados disponiveis no
                             pool.*/
};

#endif
