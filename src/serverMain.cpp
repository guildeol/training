#include <socket.h>
#include <serverSocket.h>
#include <requestHandler.h>

#include <iostream>
#include <vector>
#include <array>

#include <csignal>
#include <cstdlib>

using namespace std;

ServerSocket *server = nullptr;

vector<RequestHandler *> handler;
vector<ClientSocket *> connected;
int connections = 0;

#define CLEAN(pointer)    \
  if (pointer != nullptr) \
  {                       \
    delete pointer;       \
    pointer = nullptr;    \
  }


void cleanup(int cookie)
{
  cout << "\nInterrompendo o servidor..." << endl;

  CLEAN(server);

  for (int i = 0; i < connections; i++)
    CLEAN(handler[i]);

  for (int i = 0; i < connections; i++)
    CLEAN(connected[i]);

  return;
}

int main(int argc, char *argv[])
{
  const int backlog = 10;

  int rc[backlog];

  struct addrinfo hints;

  ClientSocket *newSocket = NULL;

  string request[backlog];
  array<bool, backlog> hasRequest;
  array<bool, backlog> done;

  string root("./");
  string port("8080");
  const int poolSize = 1024;

  char buffer[poolSize];

  string header;

  signal(SIGINT, cleanup);

  /*
   * Diretorios devem ter o '/' ao final de seus nomes. Os arquivos requisitados
   * devem ser validados em relação ao seu formato, que nao deve possuir
   * um '/' na frente.
   */
  switch (argc)
  {
    case 3:
      port.assign(argv[1]);
      root.assign(argv[2]);
      break;
    default:
      cout << "\tUso: server <porta> <raiz>" << endl;
      cout << "\tNota: Certas portas nao podem ser acessadas"
           << "por usuarios comuns!" << endl;

      return -1;
  }

  if(root.back() != '/')
    root.push_back('/');

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
    fill(connected.begin(), connected.end(), nullptr);
    handler.reserve(backlog);
    fill(handler.begin(), handler.end(), nullptr);

    hasRequest.fill(false);
    done.fill(false);

    cout << "Aguardando conexões..." << endl;

    while (true)
    {
      server->poll();

      if (server->canRead(server))
      {
        newSocket = server->accept(poolSize);

        if (connected.size() < backlog)
        {
          server->add(newSocket, POLLIN|POLLOUT);
          connected.push_back(newSocket);
          connections++;
        }
      }

      for (unsigned int i = 0; i < connected.size(); i++)
      {
        if (server->canRead(connected[i]))
        {
          // Pegar a requisição HTTP
          if (!hasRequest[i])
          {
            rc[i] = connected[i]->readLine(buffer, poolSize);

            if(rc <= 0)
              break;

            request[i].assign(buffer, rc[i]);
            handler.push_back(new RequestHandler(request[i]));

            hasRequest[i] = true;
          }

          while (true)
          {
            rc[i] = connected[i]->readLine(buffer, poolSize);

            if (rc[i] <= 0)
              break;

            handler[i]->addHeader(buffer);

            if (!strncmp(buffer, "\r\n", strlen("\r\n")))
              break;

            if (!strncmp(buffer, "\n", strlen("\n")))
              break;
          }
        }

        if (server->canSend(connected[i]) && hasRequest[i])
        {
          rc[i] = handler[i]->validate(root);

          if (rc[i] != 0)
            done[i] = handler[i]->respond(rc[i], root, connected[i]);

          if (done[i])
          {
            server->remove(connected[i]);

            CLEAN(connected[i]);
            CLEAN(handler[i]);

            handler.erase(handler.begin() + i);
            connected.erase(connected.begin() + i);

            hasRequest[i] = false;
            connections--;
          }
        }
      }
    }
  }
  catch (exception &e)
  {
    cout << e.what() << endl;
  }

  CLEAN(server);

  return 0;
}
