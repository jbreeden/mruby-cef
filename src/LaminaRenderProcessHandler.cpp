#define _CRT_SECURE_NO_DEPRECATE

#include "stdio.h"
#include "errno.h"
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby_cef.h"
#include "ruby_fn_handler.h"
#include "LaminaOptions.h"
#include "LaminaRenderProcessHandler.h"

// TODO: mrb_close? 
// May want to use a new mrb instance for each frame and close them when the context is destroyed.
// (Make that definitely... mrb isn't thread safe so we'll need a new mrb_state for each frame, which could be running javascript in parallel.)
// Need to evaluate with multiple frames.
mrb_state* LaminaRenderProcessHandler::mrb = NULL;

LaminaRenderProcessHandler::LaminaRenderProcessHandler() {
   if (mrb == NULL) {
      mrb = mrb_open();
   }
}

void LaminaRenderProcessHandler::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {
   FILE* extensions_script = fopen(LaminaOptions::script_on_v8_context_created.c_str(), "r");
   if (extensions_script != NULL) {
      mrb_load_file(mrb, extensions_script);
   }
   else {
      if (errno != ENOENT) {
         CefRefPtr<CefV8Value> ret;
         CefRefPtr<CefV8Exception> exc;
         context->Eval("alert('Could not load on_v8_context_created.rb')", ret, exc);
      }
   }

   CefRefPtr<RubyFnHandler> rubyHandler = new RubyFnHandler(mrb);
   auto ruby_fn = CefV8Value::CreateFunction("ruby", rubyHandler);
   context->GetGlobal()->SetValue(CefString("ruby"), ruby_fn, CefV8Value::PropertyAttribute::V8_PROPERTY_ATTRIBUTE_NONE);
}