#include <httpInterface.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <cstdio>
#include <cstring>

HTTPInterface::HTTPInterface(std::string &request):
  knownMethods("GET"),
  knownProtocols("HTTP/1.0 HTTP/1.1")
{
  int first = request.find(' ', 0);
  int second = request.find(' ', first + 1);
  int end = request.find_first_of("\r\n", second + 1);

  //Copia todos os caracteres antes do primeiro espaço
  method.assign(request, 0, first);

  //Copia todos os caracteres entre os espaços
  resource.assign(request, first + 1, second - first - 1);

  //Copia todos os caracteres antes do primeiro \r ou \n
  protocol.assign(request, second + 1  , end - second - 1);
}

int HTTPInterface::validate()
{
  if(this->knownMethods.find(this->method) == std::string::npos)
    return 501;

  if(this->knownProtocols.find(this->protocol) == std::string::npos)
    return 505;

  return 200;
}

int HTTPInterface::respond(int code, ClientSocket *socket)
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
    this->fetch(file, code, length);

    time_t now;
    struct tm *t;

    time(&now);
    t = gmtime(&now);

    string timeString = timeToString(*t);

    socket->send(this->protocol + " " + to_string(code) + " " + reason[code] + "\r\n");
    socket->send("Date: " + timeString + "\r\n");
    socket->send("Server: aker-training/0.1\r\n");
    socket->send("Content-Type: application/octet-stream\r\n");
    socket->send("Content-Length: " + to_string(length) + "\r\n");
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
    throw e;
  }

  return 0;
}

void HTTPInterface::fetch(std::ifstream &file, int code, int &length)
{
  using namespace std;

  if (code == 200)
    file.open(this->resource.c_str(), ios::binary);
  else
    file.open(to_string(code) + ".html", ios::binary);

  if(!file.good())
    throw runtime_error(std::string("Erro ao abrir o arquivo requisitado!"));

  file.seekg(0, file.end);
  length = file.tellg();
  file.seekg(0, file.beg);
}

std::string HTTPInterface::timeToString(struct tm &t)
{
  const int length = strlen("WWW, DDD MMM YYYY HH:MM:SS GMT");
  char timeBuffer[length];

  strftime(timeBuffer, length, "%a, %d %b %Y %T GMT", &t);

  return std::string(timeBuffer);
}
