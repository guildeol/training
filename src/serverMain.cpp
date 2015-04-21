#include <socket.h>
#include <serverSocket.h>
#include <httpInterface.h>

#include <iostream>
#include <vector>
#include <array>

using namespace std;

int main(int argc, char *argv[])
{
  const int backlog = 10;

  int rc[backlog];

  struct addrinfo hints;

  ServerSocket *server = NULL;
  ClientSocket *newSocket = NULL;

  vector<ClientSocket *> connected;
  string request[backlog];
  array<bool, backlog> hasRequest;

  string root("./");
  char *port = "80";
  const int poolSize = 1024;

  char buffer[poolSize];

  /*
   * Diretorios devem ter o '/' ao final de seus nomes. Os arquivos requisitados
   * devem ser validados, de forma a validar seu formato, que nao deve possuir
   * um '/' na frente.
   */
  try
  {
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
    server->add(server, POLLIN);

    connected.reserve(backlog);
    hasRequest.fill(false);

    cout << "Aguardando conexões..." << endl;

    while (true)
    {
      server->poll();

      if (server->canRead(server))
      {
        newSocket = server->accept(poolSize);

        //Verificar o que acontece se backlog + 1 clientes se conectarem
        if (connected.size() < backlog)
        {
          server->add(newSocket, POLLIN|POLLOUT);
          connected.push_back(newSocket);
        }
      }

      for (unsigned int i = 0; i < connected.size(); i++)
      {
        if (server->canRead(connected[i]) && !hasRequest[i])
        {
          // Pegar a requisição HTTP
          rc[i] = connected[i]->readLine(buffer, poolSize);

          if(rc <= 0)
            break;

          request[i].assign(buffer, rc[i]);

          //Lendo headers
          do
          {
            rc[i] = connected[i]->readLine(buffer, poolSize);

            if(rc[i] <= 0)
              break;
          } while (strncmp(buffer, "\r\n", strlen("\r\n")));

          hasRequest[i] = true;
        }

        if(server->canSend(connected[i]) && hasRequest[i])
        {
          HTTPInterface *analyser = new HTTPInterface(request[i]);
          rc[i] = analyser->validate();;

          if(rc[i] != 0)
            analyser->respond(rc[i], connected[i]);

          //Terminou a requisicao do cliente
          server->remove(connected[i]);
          connected.erase(connected.begin() + i);
          delete connected[i];
          delete analyser;

          hasRequest[i] = false;
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

  return 0;
}
