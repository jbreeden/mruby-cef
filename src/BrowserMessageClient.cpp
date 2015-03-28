#include "BrowserMessageClient.h"
#include "nn.h"
#include "reqrep.h"
#include "LaminaOptions.h"

using namespace std;

void 
BrowserMessageClient::set_server(string url) {
   server = url;
}

void
BrowserMessageClient::send(string message) {
   int socket = nn_socket(AF_SP, NN_REQ);
   int connection = nn_connect(socket, server.c_str());
   int send = nn_send(socket, message.c_str(), message.size() + 1, 0);
}