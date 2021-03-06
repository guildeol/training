/*!
 * \file   httpInterface.cpp
 * \brief  arquivo de implementação de um sistema que realiza analise e se
 *         comunica de acordo com o padrão HTTP
 */
#include <requestHandler.h>
#include <tokenBucket.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm> /*Para std::count*/

#include <sys/stat.h>

#include <cstdio>
#include <cstring>
#include <cerrno>

inline bool fileExists (const std::string &filename)
{
  struct stat buffer;

  bool result = (stat(filename.c_str(), &buffer) == 0);

  return result;
}

/*!
 * Construtor da classe.
 * \param[in] request string representando a requisicao feita ao servidor.
 */
RequestHandler::RequestHandler(std::string &request, std::string &root):
  method(""),
  resource(""),
  protocol("HTTP/1.0"),
  knownMethods("GET"),
  knownProtocols("HTTP/1.0 HTTP/1.1"),
  responseFolder("./responses/")
{
  using namespace std;

  this->bucket.setRate(5000000);

  int spaces = std::count(request.begin(), request.end(), ' ');

  if (spaces != 2)
    return;

  unsigned int first  = request.find_first_of(" \r\n", 0);
  unsigned int second = request.find_first_of(" \r\n", first + 1);
  unsigned int end    = request.find_first_of(" \r\n", second + 1);

  //Copia todos os caracteres antes do primeiro espaço
  this->method.assign(request, 0, first);

  //Copia todos os caracteres entre os espaços
  this->resource.assign(request, first + 1, second - first - 1);

  if(this->resource.front() == '/')
    this->resource.assign(this->resource.c_str(), 1, this->resource.size());

  //Copia todos os caracteres antes do primeiro \r ou \n
  this->protocol.assign(request, second + 1  , end - second - 1);

  this->root.assign(root);
}

/*!
 * \brief Metodo para validar os dados passado no construtor
 * \return Codigo HTTP de acordo com o avaliado.
 */
int RequestHandler::validate()
{
  using namespace std;

  if (this->method.empty() || this->resource.empty() || this->protocol.empty())
    return BAD_REQUEST;

  if (this->knownProtocols.find(this->protocol) == std::string::npos)
  {
    this->protocol.assign("HTTP/1.0");
    return NOT_SUPPORTED;
  }

  if (this->knownMethods.find(this->method) == std::string::npos)
    return NOT_IMPLEMENTED;

  if (this->resource.find("../") != std::string::npos)
    return FORBIDDEN;

  if (!fileExists(this->root + this->resource))
    return NOT_FOUND;

  // Checando os headers
  bool found = false;
  for (unsigned int i = 0; i < headers.size() && !found; i++)
    found = headers[i].find("Host: ") == 0;

  if(!found && this->protocol.compare("HTTP/1.1") == 0)
    return BAD_REQUEST;

  return OK;
}

/*!
 * \brief Adiciona uma entrada ao vetor de headers da classe, usado na validacao.
 * \param[in] header Header passado pelo servidor.
 * \throw runtime_error caso header seja nulo.
 */
void RequestHandler::addHeader(char *header)
{
  if (!header)
    throw std::runtime_error("Erro: Header nulo");

  try
  {
    std::string transfer(header);
    this->headers.push_back(transfer);
  }
  catch (std::exception &e)
  {
    std::cout << "Erro ao setar header!" << std::endl;
    throw e;
  }
}

/*!
 * \brief Envia resposta de acordo com o protocol HTTP para o socket especificado.
 * \param[in] socket Socket pelo qual a resposta deve ser enviada.
 * \throw runtime_error em caso de falha.
 */
int RequestHandler::respond(ClientSocket *socket)
{
  using namespace std;

  this->httpResponseCode = this->validate();

  const int blockSize = 1024;
  char buffer[blockSize];

  int done = 1;

  if (this->totalSent == 0)
  {
    try
    {
      this->fetch();
      this->sendHeaders(socket, this->fileLength);
    }
    catch (exception &e)
    {
      throw runtime_error(std::string("Erro ao abrir arquivo de resposta: ")
                          + strerror(errno));
    }
  }

  if(this->bucket.consume(blockSize))
  {
    try
    {
      this->file.read(buffer, blockSize);
      this->totalSent += this->file.gcount();

      socket->sendAll(buffer, this->file.gcount());
    }
    catch (exception &e)
    {
      this->totalSent = 0;
      this->file.close();
      return 0;
    }
  }
  else
    done = -1;

  bucket.replenish();

  if(this->totalSent >= this->fileLength)
  {
    this->totalSent = 0;
    this->file.close();
    done = 0;
  }

  return done;
}

/*!
 * \brief Busca o arquivo de resposta adequado, com base no codigo e nos nomes
 * contidos em this->resource.
 * \throw runtime_error em caso de falha.
 */
void RequestHandler::fetch()
{
  using namespace std;

  string fileName;

  if (this->httpResponseCode == 200)
    fileName.assign(this->root + this->resource);
  else
    fileName.assign(responseFolder + to_string(this->httpResponseCode)+ ".html");

  this->file.open(fileName, ios::binary);

  if(!this->file.good())
    throw runtime_error(std::string("Erro ao abrir o arquivo " + fileName));

  this->file.seekg(0, this->file.end);
  this->fileLength = file.tellg();
  this->file.seekg(0, this->file.beg);
}

/*!
 * \brief Formata a data e hora no formado do RFC 1123.
 * \param[int] t struct contendo os dados de data e hora.
 * \return string formatada com data.
 */
std::string RequestHandler::timeToString(struct tm &t)
{
  const int length = strlen("WWW, DDD MMM YYYY HH:MM:SS GMT");
  char timeBuffer[length];

  strftime(timeBuffer, length, "%a, %d %b %Y %T GMT", &t);

  return std::string(timeBuffer);
}

/*!
 * \brief Envia os headers da resposta para o cliente.
 * \param[int] socket Socket pelo qual os dados serão enviados.
 * \param[length] Recebe o tamanho do arquivo a ser enviado.
 */
void RequestHandler::sendHeaders(ClientSocket *socket, int fileLength)
{
  time_t now;
  struct tm *t;
  std::string timeString;

  time(&now);
  t = gmtime(&now);
  timeString = timeToString(*t);

  socket->send(this->protocol + " " + std::to_string(this->httpResponseCode) +
               " " + reason[this->httpResponseCode] + "\r\n");
  socket->send("Date: " + timeString + "\r\n");
  socket->send("Server: aker-training/0.1\r\n");

  if(this->httpResponseCode == OK)
    socket->send("Content-Type: application/octet-stream\r\n");
  else
    socket->send("Content-Type: text/html\r\n");

  socket->send("Content-Length: " + std::to_string(fileLength) + "\r\n");
  socket->send("Connection: Close\r\n");
  socket->send("\r\n");
}
