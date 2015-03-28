#define _CRT_SECURE_NO_WARNINGS

#include "mruby.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/data.h"
#include "mruby/compile.h"
#include "stdio.h"
#include "lamina_util.h"
#include "LaminaOptions.h"
#include "apr_random.h"

using namespace std;

string LaminaOptions::app_url = "";
string LaminaOptions::browser_ipc_path = "";
string LaminaOptions::cache_path = "";
string LaminaOptions::lock_file = ".lamina";
int    LaminaOptions::remote_debugging_port = 0;
char** LaminaOptions::server_args = NULL;
string LaminaOptions::server_command = "";
int    LaminaOptions::server_delay = 0;
int    LaminaOptions::server_port = 0;
string LaminaOptions::script_on_app_started = "lamina_options.rb";
string LaminaOptions::script_on_v8_context_created = "lamina_js_extensions.rb";
bool   LaminaOptions::use_page_titles = true;
string LaminaOptions::window_title = "Lamina";

#define SET_STRING_OPTION(field) \
   mrb_value param; \
   mrb_get_args(mrb, "S", &param); \
   field = mrb_str_to_cstr(mrb, param); \
   return self;

#define SET_INT_OPTION(field) \
   mrb_value param; \
   mrb_get_args(mrb, "i", &param); \
   field = param.value.i; \
   return self;

static
mrb_value lamina_load_url(mrb_state* mrb, mrb_value self) {
   SET_STRING_OPTION(LaminaOptions::app_url)
}

static
mrb_value lamina_load_server(mrb_state* mrb, mrb_value self) {
   mrb_value *arguments;
   mrb_int argc;
   mrb_get_args(mrb, "*", &arguments, &argc);

   // Allocate space for user-supplied command tokens, "-p", "PORT", and NULL
   LaminaOptions::server_args = new char*[argc + 3];
   
   // Set server_command
   LaminaOptions::server_command = mrb_str_to_cstr(mrb, *arguments);
   
   // Set server_args
   for (int i = 0; i < argc; ++i) {
      char* arg = mrb_str_to_cstr(mrb, *(arguments + i));
      LaminaOptions::server_args[i] = new char[strlen(arg) + 1];
      strcpy(LaminaOptions::server_args[i], arg);
   }

   // Append the port command
   char* port_argument = new char[10];
   sprintf(port_argument, "%d", LaminaOptions::server_port);
   LaminaOptions::server_args[argc] = "-p";
   LaminaOptions::server_args[argc + 1] = port_argument;
   LaminaOptions::server_args[argc + 2] = NULL;

   return self;
}

static
mrb_value lamina_set_window_title(mrb_state* mrb, mrb_value self) {
   SET_STRING_OPTION(LaminaOptions::window_title)
}

static
mrb_value lamina_set_cache_path(mrb_state* mrb, mrb_value self) {
   SET_STRING_OPTION(LaminaOptions::cache_path)
}

static
mrb_value lamina_set_remote_debugging_port(mrb_state* mrb, mrb_value self) {
   SET_INT_OPTION(LaminaOptions::remote_debugging_port)
}

static
mrb_value lamina_set_server_delay(mrb_state* mrb, mrb_value self) {
   SET_INT_OPTION(LaminaOptions::server_delay);
}

static
mrb_value lamina_use_page_titles(mrb_state* mrb, mrb_value self) {
   mrb_value param;
   mrb_get_args(mrb, "o", &param);
   LaminaOptions::use_page_titles = mrb_test(param);
   return self;
}

static void load_user_options() {
   mrb_state* mrb = mrb_open();

   RClass* lamina_module = mrb_define_module(mrb, "Lamina");
   mrb_define_class_method(mrb, lamina_module, "load_url", lamina_load_url, MRB_ARGS_REQ(1));
   mrb_define_class_method(mrb, lamina_module, "load_server", lamina_load_server, MRB_ARGS_ANY());
   mrb_define_class_method(mrb, lamina_module, "set_window_title", lamina_set_window_title, MRB_ARGS_REQ(1));
   mrb_define_class_method(mrb, lamina_module, "set_cache_path", lamina_set_cache_path, MRB_ARGS_REQ(1));
   mrb_define_class_method(mrb, lamina_module, "set_remote_debugging_port", lamina_set_remote_debugging_port, MRB_ARGS_REQ(1));
   mrb_define_class_method(mrb, lamina_module, "set_server_delay", lamina_set_server_delay, MRB_ARGS_REQ(1));
   mrb_define_class_method(mrb, lamina_module, "use_page_titles", lamina_use_page_titles, MRB_ARGS_REQ(1));

   FILE* lamina_options_script = fopen(LaminaOptions::script_on_app_started.c_str(), "r");
   if (lamina_options_script != NULL) {
      mrb_load_file(mrb, lamina_options_script);
      fclose(lamina_options_script);
   }

   mrb_close(mrb);
}

void LaminaOptions::load() {
   apr_pool_t* pool;
   apr_pool_create(&pool, NULL);
   LaminaOptions::server_port = lamina_util_get_open_port(pool);
   LaminaOptions::browser_ipc_path = generate_randomized_ipc_path("browser.ipc", 20);
   load_user_options();
   apr_pool_destroy(pool);
}

void LaminaOptions::load_from_lock_file() {
   auto mrb = mrb_open();
   FILE* lock_file = fopen(LaminaOptions::lock_file.c_str(), "r");
   mrb_value settings = mrb_load_file(mrb, lock_file);

#define READ_STRING_VALUE(k) \
   do { \
      mrb_value key = mrb_str_new_cstr(mrb, #k); \
      mrb_value value = mrb_funcall(mrb, settings, "[]", 1, key); \
      LaminaOptions::k = mrb_string_value_cstr(mrb, &value); \
   } while(0)

#define READ_INT_VALUE(k) \
   do { \
      mrb_value key = mrb_str_new_cstr(mrb, #k); \
      mrb_value value = mrb_funcall(mrb, settings, "[]", 1, key); \
      LaminaOptions::k = mrb_int(mrb, value); \
   } while(0)
   
   READ_STRING_VALUE(app_url);
   READ_STRING_VALUE(cache_path);
   READ_STRING_VALUE(browser_ipc_path);
   READ_INT_VALUE(remote_debugging_port);
   READ_INT_VALUE(server_port);
}


#undef SET_STRING_OPTION
#undef SET_INT_OPTION