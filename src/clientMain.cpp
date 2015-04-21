/*!
 * \file   clientMain.cpp
 * \brief  arquivo de implementação do sistema de cliente descrito no
 *         treinamento.
 */

#include <iostream>
#include <fstream>

#include <clientSocket.h>

#include <sys/stat.h>
#include <string>

#include <cstring>

using namespace std;

/*!
* \brief Verifica a existencia de um arquivo.
* \param[in] filename Nome do arquivo a ser testado.
* \return true se o arquivo existe e o usuario tem permissao,
*         false caso contrário.
*/
inline bool fileExists (const string &filename)
{
  struct stat buffer;

  bool result = (stat(filename.c_str(), &buffer) == 0);

  return result;
}

int main(int argc, char *argv[])
{
  int rc = 0;

  string address = "";
  string filename = "";
  string resource = "";

  bool overwrite = false;

  ofstream destination;

  ClientSocket *client = NULL;
  std::string port("80");
  addrinfo hints;
  const int poolSize = 1024;

  const int bufferSize = 5096;
  char buffer[bufferSize];

  int i = 0;

  switch (argc)
  {
  case 3:
    address.assign(argv[1]);
    resource.assign(argv[2]);

    for (i = resource.size(); resource[i] != '/' && i >= 0; i--)
      ;

    filename.assign(resource, i + 1, resource.size() - i);

    break;

  case 4:
    address.assign(argv[1]);
    resource.assign(argv[2]);

    if (strcmp(argv[3], "-f") == 0)
    {
      overwrite = true;

      for (i = resource.size(); resource[i] != '/' && i >= 0; i--)
        ;

      filename.assign(resource, i + 1, resource.size() - i);
    }
    else
    {
      filename.assign(argv[3]);
    }
    break;

  case 5:
    address.assign(argv[1]);
    resource.assign(argv[2]);
    filename.assign(argv[3]);

    if (strcmp(argv[4], "-f") == 0)
    {
      overwrite = true;
    }
    else
    {
      cout << "\tUso: client URI recurso [arquivo] [-f]" << endl;
      return -1;
    }
    break;

  default:
    cout << "\tUso: client URI recurso [arquivo] [-f]" << endl;
    return -1;
  }

  // Testa se o nome do recurso está no formato esperado (localizacao relativa).
  if (resource[0] != '/')
    resource = "/" + resource;

  try
  {
    if (fileExists(filename))
    {
      if (!overwrite)
      {
        cout << "Arquivo " << filename
             << " ja existe. Utilize a flag -f para forcar sobrescrita" << endl;

        return -2;
      }
    }

    /* Abre o arquivo para escrever os dados lidos.*/
    destination.open(filename, ios::out | ios::binary);

    if (!destination.good())
    {
      cout << "Erro ao abrir arquivo " << filename << " para escrita." << endl;
      return -3;
    }

    // Definindo o socket para responder ao padrão IPV4 ou IPV6, e utilizar TCP.
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    client = new ClientSocket(&address, port, hints, poolSize);

    client->connect();

    client->send("GET " + resource + " HTTP/1.0\r\n\r\n");

    /* Laco para pular os headers.*/
    while (true)
    {
      rc = client->readLine(buffer, bufferSize);

      if (rc <= 0)
        break;

      if (!strncmp(buffer, "\r\n", strlen("\r\n")))
        break;

      if (!strncmp(buffer, "\n", strlen("\n")))
        break;
    }

    while (true)
    {
      rc = client->readAll(buffer, bufferSize);

      if (rc <= 0)
        break;

      destination.write(buffer, rc);
    }

  }
  catch (exception &e)
  {
    cout << e.what() << endl;
  }

  if(destination.is_open())
    destination.close();

  if(client)
    delete client;

  return 0;
}
