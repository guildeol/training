/*!
 * \file   httpInterface.cpp
 * \brief  arquivo de implementação de um sistema que realiza analise e se
 *         comunica de acordo com o padrão HTTP
 */
#include <httpInterface.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm> /*Para std::count*/

#include <sys/stat.h>

#include <cstdio>
#include <cstring>

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
HTTPInterface::HTTPInterface(std::string &request):
  method(""),
  resource(""),
  protocol("HTTP/1.0"),
  knownMethods("GET"),
  knownProtocols("HTTP/1.0 HTTP/1.1"),
  responseFolder("./responses/")
{
  using namespace std;

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
}

/*!
 * \brief Metodo para validar os dados passado no construtor
 * \param[in] root Diretorio raiz do servidor.
 * \return Codigo HTTP de acordo com o avaliado.
 */
int HTTPInterface::validate(std::string &root)
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

  if (!fileExists(root + this->resource))
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
void HTTPInterface::addHeader(char *header)
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
 * \param[in] code Codigo da resposta, obtido via validate().
 * \param[in] root Diretorio raiz do servido.
 * \param[in] socket Socket pelo qual a resposta deve ser enviada.
 * \throw runtime_error em caso de falha.
 */
int HTTPInterface::respond(int code, std::string &root, ClientSocket *socket)
{
  using namespace std;

  ifstream file;
  int length = 0;

  const int blockSize = 512;
  char buffer[blockSize];
  int total = 0;

  int rc = 0;

  try
  {
    this->fetch(file, code, length, root);

    time_t now;
    struct tm *t;

    time(&now);
    t = gmtime(&now);

    string timeString = timeToString(*t);

    socket->send(this->protocol + " " + to_string(code) + " " + reason[code]
                 + "\r\n");
    socket->send("Date: " + timeString + "\r\n");
    socket->send("Server: aker-training/0.1\r\n");

    if(code == OK)
      socket->send("Content-Type: application/octet-stream\r\n");
    else
      socket->send("Content-Type: text/html\r\n");

    socket->send("Content-Length: " + to_string(length) + "\r\n");
    socket->send("Connection: Close\r\n");
    socket->send("\r\n");

    do
    {
      file.read(buffer, blockSize);
      total += file.gcount();

      rc = socket->sendAll(buffer, file.gcount());

      if(rc != 0)
        throw runtime_error(string("Erro ao enviar arquivo para cliente!"));

    } while(total < length);
  }
  catch (exception &e)
  {
    cout << "Erro ao enviar o arquivo de resposta!" << endl;
    throw e;
  }

  return 0;
}

/*!
 * \brief Busca o arquivo de resposta adequado, com base no codigo e nos nome contido
 * em this->resource.
 * \param[out] file referencia para o objeto com os dados do arquivo a ser
 *                  enviado.
 * \param[in] code Codigo da resposta, obtido via validate().
 * \param[length] Recebe o tamanho do arquivo a ser enviado.
 * \param[in] root Diretorio raiz do servidor.
 * \throw runtime_error em caso de falha.
 */
void HTTPInterface::fetch(std::ifstream &file, int code, int &length,
                          std:: string &root)
{
  using namespace std;

  if (code == 200)
    file.open(root + this->resource, ios::binary);
  else
    file.open(responseFolder + to_string(code)+ ".html", ios::binary);

  if(!file.good())
    throw runtime_error(std::string("Erro ao abrir o arquivo de resposta!"));

  file.seekg(0, file.end);
  length = file.tellg();
  file.seekg(0, file.beg);
}

/*!
 * \brief Formata a data e hora no formado do RFC 1123.
 * \param[int] t referencia para struct contendo os dados de data e hora.
 * \return string formatada com data.
 */
std::string HTTPInterface::timeToString(struct tm &t)
{
  const int length = strlen("WWW, DDD MMM YYYY HH:MM:SS GMT");
  char timeBuffer[length];

  strftime(timeBuffer, length, "%a, %d %b %Y %T GMT", &t);

  return std::string(timeBuffer);
}
