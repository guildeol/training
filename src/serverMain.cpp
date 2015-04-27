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

  for (unsigned int i = 0; i < handler.size(); i++)
    CLEAN(handler[i]);

  for (unsigned int i = 0; i < connected.size(); i++)
    CLEAN(connected[i]);

  return;
}

int main(int argc, char *argv[])
{
  const int backlog = 10;

  int rc[backlog];

  struct addrinfo hints;

  ClientSocket *newSocket = NULL;

  string request;
  vector<bool> hasRequest;
  bool done;

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
    hasRequest.reserve(backlog);
    fill(hasRequest.begin(), hasRequest.end(), false);
  }
  catch (exception &e)
  {
    cout << "Erro durante a inicialização do servidor..." << endl;
    cout << e.what() << endl;
    cout << "Saindo..." << endl;
    return -2;
  }

  cout << "Aguardando conexões..." << endl;

  bool bad = false;

  try
  {
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
          hasRequest.push_back(false);
        }
      }

      for (unsigned int i = 0; i < connected.size(); i++)
      {
        server->poll();

        if (server->canRead(connected[i]))
        {
          // Pegar a requisição HTTP
          rc[i] = connected[i]->readLine(buffer, poolSize);

          if(rc <= 0)
          {
            bad = true;
            break;
          }

          request.assign(buffer, rc[i]);
          handler.push_back(new RequestHandler(request));
          cout << "Got request: " << request << endl;

          while (true)
          {
            rc[i] = connected[i]->readLine(buffer, poolSize);

            if (rc[i] <= 0)
            {
              bad = true;
              break;
            }

            handler[i]->addHeader(buffer);

            if (!strncmp(buffer, "\r\n", strlen("\r\n")))
              break;

            if (!strncmp(buffer, "\n", strlen("\n")))
              break;
          }

          if (!bad)
          {
            rc[i] = handler[i]->validate(root);
            hasRequest[i] = true;
          }
        }

        if (server->canSend(connected[i]) && hasRequest[i] && !bad)
        {
          if (rc[i] != 0)
            done = handler[i]->respond(rc[i], root, connected[i]);
        }

        if (done || bad)
        {
          server->remove(connected[i]);

          CLEAN(connected[i]);
          CLEAN(handler[i]);

          handler.erase(handler.begin() + i);
          connected.erase(connected.begin() + i);
          hasRequest.erase(hasRequest.begin() + i);
          done = false;
          bad = false;
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
