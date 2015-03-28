#ifndef LAMINA_OPTIONS_H
#define LAMINA_OPTIONS_H

#include <stdlib.h>
#include <string>
#include <list>

class LaminaOptions {
public:
   static std::string app_url;
   static std::string browser_ipc_path;
   static std::string cache_path;
   static std::string lock_file;
   static int remote_debugging_port;
   static char** server_args;
   static std::string server_command;
   static int server_delay;
   static int server_port;
   static std::string script_on_app_started;
   static std::string script_on_v8_context_created;
   static bool use_page_titles;
   static std::string window_title;

   static void load();
   static void load_from_lock_file();
};

#endif /* LAMINA_OPTIONS_H */