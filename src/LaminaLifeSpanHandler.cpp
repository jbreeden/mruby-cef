#include "LaminaLifeSpanHandler.h"
#include "LaminaOptions.h"
#include <vector>
#include <regex>
#include "Windows.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
//#include "Resource.h"

using namespace std;

namespace {
   LaminaLifeSpanHandler* g_instance = NULL;
}

BrowserList
LaminaLifeSpanHandler::browser_list_ = BrowserList();

LaminaLifeSpanHandler::LaminaLifeSpanHandler()
   : is_closing_(false) {
   DCHECK(!g_instance);
}

LaminaLifeSpanHandler::~LaminaLifeSpanHandler() {
   g_instance = NULL;
}

// static
LaminaLifeSpanHandler*
LaminaLifeSpanHandler::GetInstance() {
   if (g_instance == NULL) {
      g_instance = new LaminaLifeSpanHandler;
   }
   return g_instance;
}

void
LaminaLifeSpanHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
   CEF_REQUIRE_UI_THREAD();

   // Set the frame window title bar
   CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
   SetWindowTextA(hwnd, LaminaOptions::window_title.c_str());

   // Add to the list of existing browsers.
   browser_list_.push_back(browser);

#ifdef _WIN32
   /*const HICON hicon = ::LoadIcon(
      ::GetModuleHandle(0),
      MAKEINTRESOURCE(IDI_LAMINA)
      );
   SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);*/
#endif

}

bool
LaminaLifeSpanHandler::DoClose(CefRefPtr<CefBrowser> browser) {
   CEF_REQUIRE_UI_THREAD();

   // Closing the main window requires special handling. See the DoClose()
   // documentation in the CEF header for a detailed destription of this
   // process.
   if (browser_list_.size() == 1) {
      // Set a flag to indicate that the window close should be allowed.
      is_closing_ = true;
   }

   // Allow the close. For windowed browsers this will result in the OS close
   // event being sent.
   return false;
}

void
LaminaLifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
   CEF_REQUIRE_UI_THREAD();

   // Remove from the list of existing browsers.
   BrowserList::iterator bit = browser_list_.begin();
   for (; bit != browser_list_.end(); ++bit) {
      if ((*bit)->IsSame(browser)) {
         browser_list_.erase(bit);
         break;
      }
   }

   if (browser_list_.empty()) {
      // All browser windows have closed. Quit the application message loop.
      CefQuitMessageLoop();
   }
}

void
LaminaLifeSpanHandler::CloseAllBrowsers(bool force_close) {
   if (!CefCurrentlyOn(TID_UI)) {
      // Execute on the UI thread.
      CefPostTask(TID_UI,
         base::Bind(&LaminaLifeSpanHandler::CloseAllBrowsers, this, force_close));
      return;
   }

   if (browser_list_.empty())
      return;

   BrowserList::const_iterator it = browser_list_.begin();
   for (; it != browser_list_.end(); ++it)
      (*it)->GetHost()->CloseBrowser(force_close);
}

void LaminaLifeSpanHandler::ExecuteJavaScript(char* script, char* window_pattern, bool firstMatch) {
   auto browsers = browser_list_;
   regex regexp(window_pattern);
   for (auto browserPtr = browsers.begin(); browserPtr != browsers.end(); ++browserPtr) {
      auto browser = *browserPtr;
      vector<CefString> frameNames;
      browser->GetFrameNames(frameNames);
      for (auto frameNamePtr = frameNames.begin(); frameNamePtr != frameNames.end(); ++frameNamePtr){
         auto name = *frameNamePtr;
         if (regex_match(name.ToString(), regexp)) {
            auto frame = browser->GetFrame(name);
            frame->ExecuteJavaScript(script, frame->GetURL(), 0);
            if (firstMatch) {
               return;
            }
         }
      }
   }
}
