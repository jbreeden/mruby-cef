#include <string>
#include "lamina_util.h"
#include "LaminaOptions.h"

using namespace std;

/* Networking */

apr_socket_t* lamina_util_open_tcp_socket(apr_port_t port, apr_pool_t* pool) {
   apr_socket_t *socket;
   apr_socket_create(&socket, APR_INET, SOCK_STREAM, APR_PROTO_TCP, pool);
   apr_sockaddr_t *addr;
   apr_sockaddr_info_get(&addr, NULL, APR_INET, port, 0, pool);
   apr_socket_bind(socket, addr);
   return socket;
}

int lamina_util_get_open_port(apr_pool_t* pool) {
   apr_socket_t* socket = lamina_util_open_tcp_socket(0, pool);
   apr_sockaddr_t *addr;
   apr_socket_addr_get(&addr, apr_interface_e::APR_LOCAL, socket);
   apr_socket_close(socket);
   return addr->port;
}

string generate_randomized_ipc_path(string suffix, int random_str_len) {
   unsigned char* bytes = new unsigned char[random_str_len + 1];
   apr_generate_random_bytes(bytes, random_str_len);

   char *characters = new char[random_str_len + 1];
   for (int i = 0; i < random_str_len; ++i) {
      characters[i] = (bytes[i] % 26) + 'a';
   }
   characters[random_str_len] = '\0';

   return string("ipc://") + string(characters) + string("/") + suffix;
}

/* Files */

void lamina_util_get_shared_lock(apr_pool_t* pool) {
   apr_file_t* file;
   apr_file_open(&file, LaminaOptions::lock_file.c_str(), APR_READ, NULL, pool);
   apr_status_t lock_status = apr_file_lock(file, APR_FLOCK_SHARED);
}