#include <thread>
#include "BrowserMessageServer.h"
#include "nn.h"
#include "reqrep.h"
#include "LaminaOptions.h"
#include "LaminaHandler.h"

using namespace std;

void
BrowserMessageServer::start() {
   thread* browser_ipc_thread = new thread([&](){
      this->socket = nn_socket(AF_SP, NN_REP);
      nn_bind(this->socket, LaminaOptions::browser_ipc_path.c_str());

      while (true) {
         void* buf = NULL;
         int nbytes = nn_recv(this->socket, &buf, NN_MSG, 0);
         if (nbytes > 0) {
            LaminaLifeSpanHandler::GetInstance()->ExecuteJavaScript("window.open('/');", ".*", true);
         }
         nn_freemsg(buf);
      }
   });

   browser_ipc_thread->detach();
}