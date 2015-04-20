/*!
* \file serverSocket.h
* \brief Cabecalho para classe com funcionalidades para criacao de servidor
*        utilizando como base Socket.
*
* \date 15/04/2015
* \author Guilherme Costa <glhrmcosta91@gmail.com>
*/

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <socket.h>
#include <clientSocket.h>
#include <poll.h>

#include <string>

/*!
* \brief Classe para criacao de um servidor utilizando sockets.
* \class ServerSocket
*/

class ServerSocket : public Socket
{

public:

  /*!
   * Construtor da classe, prepara as estruturas necessarias para conexao
   * atraves do socket utilizando por padrao conexao TCP e a porta 80.
   * \param[in] address endereco do servidor desejado.
   * \param[in] poolSize tamanho do pool que deve ser reservado para leitura.
   * \param[in] port Numero da porta na qual a socket deve se comunicar
   * \param[in] maxDescriptors Quantidade de descritores que pode ser "vigiada"
   *            com poll().
   * \throw runtime_error ao chamar socket.
   * \throw runtime_error ao chamar getaddrinfo.
   * \throw bad_alloc caso a alocação de pool falhar.
   */
  ServerSocket(const std::string *address, char *port, const addrinfo &hints,
                int poolSize = 0, int maxDescriptors = 0);

  ~ServerSocket();

  /*!
  * \brief Realiza um bind do socket na porta especificada no construtor
  * \param[in] reuseAddress Variavel indicando se o sistema deve forcar
                            a reutilizacao do endereço
  * \return Retorna 0 em caso de sucesso
  * \throw runtime_error caso o bind falhe
  */
  int bind(bool reuseAddress = true);

  /*!
  * \brief Faz com que o socket fique escutando por requisições
  * \param[in] backlog Quantidade maxima de requisições pendentes
  * \return Retorna 0 em caso de sucesso
  * \throw runtime_error caso listen falhe
  */
  int listen(const int backlog);

  /*!
  * \brief Aceita uma conexão pendente
  * \param[in] poolSize Tamanho do pool do novo socket a ser retornado
  * \return Retorna um novo Socket para comunicação em caso de sucesso
  * \throw runtime_error caso accept falhe
  */
  ClientSocket* accept(const int poolSize = 0);

  /*!
  * \brief Adiciona o descritor de socket à lista de descritores "vigiados"
  por poll.
  * \param[in] socket Socket que terá seu descritor adicionado.
  * \return Retorna um novo Socket para comunicação em caso de sucesso
  * \throw invalid_argument caso socket seja nulo.
  * \throw length_error caso o limite de descritores tenha sido alcancado.
  */
  int add(Socket *socket, int events);

  /*!
  * \brief Remove o descritor de socket da lista de descritores "vigiados"
  por poll.
  * \param[in] socket Socket que terá seu descritor removido.
  * \return Retorna um novo Socket para comunicação em caso de sucesso
  * \throw invalid_argument caso socket seja nulo.
  * \throw length_error caso não exista mais nenhum descritor a ser removido.
  */
  int remove(Socket *socket);

  /*!
  * \brief Avalia o resultado de poll para saber se um socket esta apto a receber
  *         dados (enviados atraves de send()).
  * \param[in] socket Socket a ser avaliado.
  * \return true se o socket estiver apto, false caso contrario.
  * \throw invalid_argument caso socket seja nulo.
  */
  bool canSend(Socket *socket);

  /*!
  * \brief Avalia o resultado de poll para saber se um socket possui dados
  *        esperando para serem lidos (com receive(), readLine() ou readAll()).
  * \param[in] socket Socket a ser avaliado.
  * \return true se o socket possuir dados, false caso contrario.
  * \throw invalid_argument caso socket seja nulo.
  */
  bool canRead(Socket *socket);

  /*!
  * \brief Realiza o poll dos descritores contidos em descriptors.
  * \param[in] timeout Tempo de espera (em ms). Se timeout < 0, a funcao aguarda
  *            ate que um descritor esteja pronto, se timeout == 0, a funcao
  *            retorna imediatamente.
  * \return O numero de descritores que tiverem seu estado modificado.
  * \throw runtime_error caso poll falhe.
  */
  int poll(int timeout = -1);

private:

  struct sockaddr_storage endpoint; /*!< Utilizado em accept().*/

  /*!
  * Array de structs utilizado em poll. Deve ser mantido pelo
  * listerner/servidor.
  */
  struct pollfd *descriptors = NULL;
  int currentDescriptors = 0;
  int maxDescriptors = 0;
};

#endif
