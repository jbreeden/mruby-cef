/* System Includes */
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <cstdio>
#include <thread>
#include <chrono>
#include "Shobjidl.h"

/* CEF Includes */
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_client.h"
#include "include/cef_sandbox_win.h"
#include "include/cef_runnable.h"

/* APR Includes */
#include "apr_file_io.h"
#include "apr_thread_proc.h"
#include "apr_network_io.h"

/* MRuby Includes */
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/irep.h"

/* Lamina Includes */
#include "Lamina.h"
#include "lamina_util.h"
#include "LaminaHandler.h"
#include "LaminaApp.h"
#include "LaminaOptions.h"
#include "BrowserMessageServer.h"
#include "BrowserMessageClient.h"

using namespace std;

// Notes on sandboxing:
//  - On windows, this currently only works if the sub-process exe is the same exe as the main process
//  - With sandboxing enabled, render processes cannot access system resources like networking/files
//  - Currently running without sandboxing to allow the render process to load mruby scripts
//    + This also allows javascript extensions to access resources without going through IPC
//    + This isn't great from a security perspective. Will need to work around this and re-enable sandboxing
//      as soon as a decent story is available for creating JS extensions with mruby scripts in a sandboxed environment.
//#ifdef _WIN32
//#define CEF_USE_SANDBOX
//#endif

#if __cplusplus
extern "C" {
#endif

void lamina_ensure_lock_file_exists() {
   auto f = fopen(LaminaOptions::lock_file.c_str(), "r");
   if (f == NULL && errno == ENOENT) {
      f = fopen(LaminaOptions::lock_file.c_str(), "w");
      fclose(f);
   }
}

void try_with_exclusive_lock(std::function<void(apr_file_t*)> success, std::function<void()> failure) {
   apr_file_t* lock_file;
   apr_pool_t* pool;
   apr_pool_create(&pool, NULL);
   apr_file_open(&lock_file, LaminaOptions::lock_file.c_str(), APR_FOPEN_WRITE, APR_FPROT_OS_DEFAULT, pool);
   if (apr_file_lock(lock_file, APR_FLOCK_EXCLUSIVE | APR_FLOCK_NONBLOCK) == APR_SUCCESS) {
      success(lock_file);
      apr_file_unlock(lock_file);
      apr_file_close(lock_file);
   }
   else {
      apr_file_close(lock_file);
      failure();
   }
}

void lamina_write_app_settings(apr_file_t* lock_file) {
   // APR_FOPEN_TRUNCATE flag seems to prevent writes from taking effect.
   // So, just truncate manually before writing.
   apr_file_trunc(lock_file, 0);
   apr_file_printf(lock_file, "{\n");
   apr_file_printf(lock_file, "  'app_url' => '%s',\n", LaminaOptions::app_url.c_str());
   apr_file_printf(lock_file, "  'browser_ipc_path' => '%s',\n", LaminaOptions::browser_ipc_path.c_str());
   apr_file_printf(lock_file, "  'cache_path' => '%s',\n", LaminaOptions::cache_path.c_str());
   apr_file_printf(lock_file, "  'remote_debugging_port' => %d,\n", LaminaOptions::remote_debugging_port);
   apr_file_printf(lock_file, "  'server_port' => %d,\n", LaminaOptions::server_port);
   apr_file_printf(lock_file, "}\n");
}

void lamina_rename_local_storage_files() {
   auto mrb = mrb_open();

   char script[300];
   sprintf(script,
     "if Dir.exists? '%s/Local Storage' \n"
     "  Dir.chdir('%s/Local Storage') do \n"
     "    Dir.entries('.').each do |f| \n"
     "      if m = f.match(/^http_localhost_([0-9]*).localstorage/i) \n"
     "        File.rename f, f.sub(/localhost_([0-9]*)/, 'localhost_%d') \n"
     "      end \n"
     "    end \n"
     "  end \n"
     "end",
   LaminaOptions::cache_path.c_str(),
   LaminaOptions::cache_path.c_str(),
   LaminaOptions::server_port);

   mrb_load_string(mrb, script);
   mrb_close(mrb);
}

void lamina_launch_server(apr_pool_t* pool) {
   apr_procattr_t* procattr;
   apr_procattr_create(&procattr, pool);
   apr_procattr_cmdtype_set(procattr, APR_PROGRAM_PATH);
   apr_proc_t proc;
   apr_proc_create(&proc, LaminaOptions::server_command.c_str(), LaminaOptions::server_args, NULL, procattr, pool);
   if (LaminaOptions::server_delay) {
      std::this_thread::sleep_for(chrono::seconds(LaminaOptions::server_delay));
   }
}

int lamina_start_cef() {
#ifdef DEBUG
   cout << "New Main Process" << endl;
#endif

   void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
   // Manage the life span of the sandbox information object. This is necessary
   // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
   CefScopedSandboxInfo scoped_sandbox;
   sandbox_info = scoped_sandbox.sandbox_info();
#endif

   // Provide CEF with command-line arguments.
   CefMainArgs main_args;
   // SimpleApp implements application-level callbacks. It will create the first
   // browser instance in OnContextInitialized() after CEF has initialized.
   CefRefPtr<LaminaApp> app(new LaminaApp);

   // If a server command is present, it takes precedent over the app_url.
   // Client *should* be calling either `load_url` or `load_server`,
   // so only one should be populated.
   if (LaminaOptions::server_command.size() > 0) {
      char server_url[100];
      sprintf(server_url, "http://localhost:%d", LaminaOptions::server_port);
      app.get()->url = server_url;
   }
   else {
      app.get()->url = LaminaOptions::app_url;
   }

#ifdef DEBUG
   cout << "APP URL: " << app.get()->url << endl;
#endif

   // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
   // that share the same executable. This function checks the command-line and,
   // if this is a sub-process, executes the appropriate logic.
   int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
   if (exit_code >= 0) {
      // The sub-process has completed so return here.
      return exit_code;
   }

   // Specify CEF global settings here.
   CefSettings settings;

#ifndef _DEBUG /* Suppresses debug.log output when building in release mode */
   settings.log_severity = LOGSEVERITY_DISABLE;
#endif

   if (strlen(LaminaOptions::cache_path.c_str()) > 0) {
      CefString(&settings.cache_path).FromASCII(LaminaOptions::cache_path.c_str());
   }

   settings.remote_debugging_port = LaminaOptions::remote_debugging_port;

#if !defined(CEF_USE_SANDBOX)
   settings.no_sandbox = true;
#endif

   // Initialize CEF.
   CefInitialize(main_args, settings, app.get(), sandbox_info);

   // Run the CEF message loop. This will block until CefQuitMessageLoop() is
   // called.
   CefRunMessageLoop();

   // Shut down CEF.
   CefShutdown();

   return 0;
}

int lamina_start(int argc, char** argv) {
   apr_initialize();
   apr_pool_t* pool;
   apr_pool_create(&pool, NULL);

   lamina_ensure_lock_file_exists();
   try_with_exclusive_lock(
      /* Got the lock (this is the first app instance) */
      [&](apr_file_t* file) {
         LaminaOptions::load();
         lamina_write_app_settings(file);
         if (LaminaOptions::cache_path.size() > 0) {
            lamina_rename_local_storage_files();
         }
         if (LaminaOptions::server_command.size() > 0) {
            lamina_launch_server(pool);
         }
         auto browserMessageServer = new BrowserMessageServer();
         browserMessageServer->start();
      },
      /* Couldn't get the lock (app is running already) */
      [&](){
         if (argc == 1) {
            // CEF will supply a bunch of arguments for sub-processes (renderers, gpu procs, etc.)
            // If there are no arguments, this is an attempt to re-launch the application by the user.
            // In that case, simply send a message to the application to open a new window
            LaminaOptions::load_from_lock_file();
            BrowserMessageClient client;
            client.set_server(LaminaOptions::browser_ipc_path);
            client.send("new_window");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            exit(0);
         }
      }
   );
   lamina_util_get_shared_lock(pool);

   return lamina_start_cef();
}

#if __cplusplus
}
#endif