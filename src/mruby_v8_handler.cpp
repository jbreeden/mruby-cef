#include "mruby_v8_handler.h"
#include "mruby_cef.h"
#include "include\cef_v8.h"

using namespace std;

MRubyV8Handler::MRubyV8Handler(mrb_state* mrb, string name, mrb_value block) {
   this->mrb = mrb;
   this->name = name;
   this->block = block;
}

bool 
MRubyV8Handler::Execute(const CefString& name,
   CefRefPtr<CefV8Value> object,
   const CefV8ValueList& arguments,
   CefRefPtr<CefV8Value>& retval,
   CefString& exception) {

   if (name == this->name) {
      
      CefRefPtr<CefV8Value> args = CefV8Value::CreateArray(0);
      CefRefPtr<CefV8Value> push_fn = args->GetValue(CefString("push"));

      for (auto it = arguments.begin(); it != arguments.end(); ++it) {
         push_fn->ExecuteFunction(args, { *it });
      }

      mrb_value rb_args = mrb_cef_v8_value_wrap(this->mrb, args);
      mrb_value ret = mrb_funcall(this->mrb, this->block, "call", 1, rb_args);
      if (strcmp(mrb_obj_classname(this->mrb, ret), "JsObject")) {
         retval = mrb_cef_v8_value_unwrap(mrb, ret);
      }
      else {
         retval = CefV8Value::CreateUndefined();
      }
      
      return true;
   }

   // Function does not exist.
   return false;
}