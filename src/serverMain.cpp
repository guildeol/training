#include <socket.h>
#include <serverSocket.h>
#include <requestHandler.h>

#include <iostream>
#include <vector>
#include <array>

#include <csignal>
#include <cstdlib>

using namespace std;

#define GETREQUEST 0
#define GETHEADERS 1
#define SEND       2
#define END        3

ServerSocket *server = nullptr;

vector<RequestHandler *> requestHandlers;
vector<ClientSocket *> clients;
vector<int> clientStates;
vector<int> httpResponse;

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

  for (unsigned int i = 0; i < requestHandlers.size(); i++)
    CLEAN(requestHandlers[i]);

  for (unsigned int i = 0; i < clients.size(); i++)
    CLEAN(clients[i]);

  return;
}

int main(int argc, char *argv[])
{
  const int backlog = 200;

  struct addrinfo hints;

  ClientSocket *newSocket = NULL;

  string request;
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
    // Inicializando os vetores com variaveis de controle
    requestHandlers.reserve(backlog);
    fill(requestHandlers.begin(), requestHandlers.end(), nullptr);

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

  }
  catch (exception &e)
  {
    cout << "Erro durante a inicialização do servidor..." << endl;
    cout << e.what() << endl;
    cout << "Saindo..." << endl;
    return -2;
  }

  cout << "Aguardando conexões..." << endl;

  try
  {
    while (true)
    {
      server->poll(0);

      if (server->canRead(server))
      {
        newSocket = server->accept(poolSize);
        server->add(newSocket, POLLIN);
        clients.push_back(newSocket);
        clientStates.push_back(GETREQUEST);
      }

      for (unsigned int i = 0; i < clients.size(); i++)
      {
        int bytesRead = 0;

        switch (clientStates[i])
        {
          case GETREQUEST:
            if (server->canRead(clients[i]))
            {
              bytesRead = clients[i]->readLine(buffer, poolSize);

              if(bytesRead <= 0)
                clientStates[i] = END;

              request.assign(buffer, bytesRead);
              requestHandlers.push_back(new RequestHandler(request, root));
              cout << "Got request: " << request << endl;

              clientStates[i] = GETHEADERS;
            }
            break;

          case GETHEADERS:
            bytesRead = clients[i]->readLine(buffer, poolSize);

            if(bytesRead <= 0)
              clientStates[i] = END;

            requestHandlers[i]->addHeader(buffer);

            if (!strncmp(buffer, "\r\n", strlen("\r\n")))
              clientStates[i] = SEND;

            if (!strncmp(buffer, "\n", strlen("\n")))
              clientStates[i] = SEND;

            break;

          case SEND:
            done = requestHandlers[i]->respond(clients[i]);

            if (done)
              clientStates[i] = END;
            break;

          case END:

          server->remove(clients[i]);
            CLEAN(clients[i]);
            CLEAN(requestHandlers[i]);

            requestHandlers.erase(requestHandlers.begin() + i);
            clients.erase(clients.begin() + i);
            clientStates.erase(clientStates.begin() + i);
            break;
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
