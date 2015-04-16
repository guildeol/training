#include <httpInterface.h>

#include <iostream>

HTTPInterface::HTTPInterface(std::string &request)
{
  int first = request.find(' ', 0);
  int second = request.find(' ', first + 1);
  int end = request.find_first_of("\r\n", second + 1);

  //Copia todos os caracteres antes do primeiro espaço
  method.assign(request, 0, first);

  //Copia todos os caracteres entre os espaços
  resource.assign(request, first + 1, second - first - 1);

  //Copia todos os caracteres antes do primeiro \r ou
  protocol.assign(request, second + 1  , end - second - 1);
}

int HTTPInterface::respond(int code, Socket &socket)
{
  return 0;
}
