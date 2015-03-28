#ifndef RUBYCHROMEAPP_H
#define RUBYCHROMEAPP_H

#include <string>
#include "include/cef_app.h"

class LaminaApp : public CefApp,
   public CefBrowserProcessHandler {
public:
   LaminaApp();

   // CefApp methods:
   virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE 
   { 
      return this; 
   }

   virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE;

   // CefBrowserProcessHandler methods:
   virtual void OnContextInitialized() OVERRIDE;

   std::string url;

private:
   // Include the default reference counting implementation.
   IMPLEMENT_REFCOUNTING(LaminaApp);
};

#endif /* RUBYCHROMEAPP_H */