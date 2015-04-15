#include <socketWrapper.h>
#include <serverSocket.h>

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
  const int backlog = 10;

  struct addrinfo hints;

  ServerSocket *server = NULL;
  SocketWrapper *newSocket = NULL;

  // char buffer[1024];
  // int rc = 0;

  try
  {
    char *port = "8080";
    int poolSize = 1024;

    // Conexao tipo TCP, IPV4 ou V6, preenchimento automatico de IP
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    server = new ServerSocket(NULL, port, hints, poolSize, backlog);
  }
  catch (exception e)
  {
    cout << e.what() << endl;
  }

  return 0;
}
