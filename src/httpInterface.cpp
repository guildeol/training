#include <httpInterface.h>

#include <fstream>
#include <stdexcept>

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

  return 0;
}

int HTTPInterface::respond(int code, Socket *socket)
{
  // using namespace std;
  //
  // ifstream file;
  // int length;
  //
  // try
  // {
  //   file = fetch(code, length);
  // }
  // catch (exception &e)
  // {
  //   throw runtime_error(std::string("Erro ao abrir o arquivo requisitado!"));
  // }
  //
  // socket->send(protocol + " " + code + " " + reason + "\r\n");
  // socket->send("Date: " + date + "\r\n");
  // socket->send("Server: aker-training/0.1\r\n");
  // socket->send("Content-Type: " + type + "\r\n");
  // socket->send("Content-Length: " + length + "\r\n");
  // socket->send("\r\n");
  //
  // socket->sendAll(file);

  return 0;
}

// std::ifstream fetch(int code, int &length)
// {
//   // using namespace std;
//   //
//   // fstream file;
//   //
//   // if (code == 200)
//   //   file.open(resource, ios::binary);
//   // else
//   //   file.open(responseFiles[code], ios::binary);
//   //
//   // if(!file.is_good())
//   //   throw runtime_error(std::string("Erro ao abrir o arquivo requisitado!"));
//   //
//   // file.seekg(0, file.end);
//   // length = file.tellg();
//   // file.seekg(0, file.begin);
//   //
//   // return file;
// }
