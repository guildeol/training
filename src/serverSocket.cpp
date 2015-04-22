#include <iostream>
#include <serverSocket.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

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
ServerSocket::ServerSocket(const std::string *address, std::string port,
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

/*!
* \brief Realiza um bind do socket na porta especificada no construtor
* \param[in] reuseAddress Variavel indicando se o sistema deve forcar
                          a reutilizacao do endereço
* \return Retorna 0 em caso de sucesso
* \throw runtime_error caso o bind falhe
*/
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

/*!
* \brief Faz com que o socket fique escutando por requisições
* \param[in] backlog Quantidade maxima de requisições pendentes
* \return Retorna 0 em caso de sucesso
* \throw runtime_error caso listen falhe
*/
int ServerSocket::listen(const int backlog)
{
  int rc = ::listen(this->socketDescriptor, backlog);

  if( rc == -1)
    throw std::runtime_error(std::string("Erro em listen: ") + strerror(errno));

  return rc;
}

/*!
* \brief Aceita uma conexão pendente
* \param[in] poolSize Tamanho do pool do novo socket a ser retornado
* \return Retorna um novo ClientSocket para comunicação em caso de sucesso
* \throw runtime_error caso accept falhe
*/
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

/*!
* \brief Adiciona o descritor de socket à lista de descritores "vigiados"
*        por poll.
* \param[in] socket Socket que terá seu descritor adicionado.
* \return Retorna um novo Socket para comunicação em caso de sucesso
* \throw invalid_argument caso socket seja nulo.
* \throw length_error caso o limite de descritores tenha sido alcancado.
*/
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

/*!
* \brief Remove o descritor de socket da lista de descritores "vigiados"
*        por poll.
* \param[in] socket Socket que terá seu descritor removido.
* \return Retorna um novo Socket para comunicação em caso de sucesso
* \throw invalid_argument caso socket seja nulo.
* \throw length_error caso não exista mais nenhum descritor a ser removido.
*/
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

/*!
* \brief Realiza o poll dos descritores contidos em descriptors.
* \param[in] timeout Tempo de espera (em ms). Se timeout < 0, a funcao aguarda
*            ate que um descritor esteja pronto, se timeout == 0, a funcao
*            retorna imediatamente.
* \return O numero de descritores que tiverem seu estado modificado.
* \throw runtime_error caso poll falhe.
*/
int ServerSocket::poll(int timeout)
{
  int rc = ::poll(this->descriptors, this->maxDescriptors, timeout);

  if(rc == -1)
    throw std::runtime_error(std::string("Erro em poll: ") + strerror(errno));

  return rc;
}


/*!
* \brief Avalia o resultado de poll para saber se um socket possui dados
*        esperando para serem lidos (com receive(), readLine() ou readAll()).
* \param[in] socket Socket a ser avaliado.
* \return true se o socket possuir dados, false caso contrario.
* \throw invalid_argument caso socket seja nulo.
*/
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

/*!
* \brief Avalia o resultado de poll para saber se um socket esta apto a receber
*         dados (enviados atraves de send()).
* \param[in] socket Socket a ser avaliado.
* \return true se o socket estiver apto, false caso contrario.
* \throw invalid_argument caso socket seja nulo.
*/
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
