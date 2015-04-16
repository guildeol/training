#include <socket.h>
#include <serverSocket.h>

#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
  struct addrinfo hints;

  ServerSocket *server = NULL;
  Socket *newSocket = NULL;

  const int backlog = 1;

  vector<Socket *> connected;

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

    /*
     * Sao reservados backlog + 1 descritores, pois o servidor deve utilizar um
     * destes descritores.
     */
    server = new ServerSocket(NULL, port, hints, poolSize, backlog + 1);

    server->bind();
    server->listen(backlog);

    // O socket do servidor sempre e adicionado primeiro no array de descritores
    server->add(server, POLLIN);

    connected.reserve(backlog);

    cout << "Aguardando conexões..." << endl;

    while (true)
    {
      server->poll();

      if (server->canRead(server))
      {
        newSocket = server->accept(poolSize);

        server->add(newSocket, POLLIN|POLLOUT);

        if (connected.size() < backlog)
          connected.push_back(newSocket);
      }

      for (unsigned int i = 0; i < connected.size(); i++)
      {
        if(server->canSend(connected[i]))
        {
          connected[i]->send("Hey there!\n");

          server->remove(connected[i]);
          connected.erase(connected.begin() + i);
          delete connected[i];
        }
      }
    }
  }
  catch (exception &e)
  {
    cout << e.what() << endl;
  }

  if(server)
    delete server;

  if(newSocket)
    delete newSocket;

  return 0;
}
