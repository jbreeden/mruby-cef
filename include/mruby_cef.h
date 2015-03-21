#ifndef MRUBY_CEF_V8_H
#define MRUBY_CEF_V8_H

#include "mruby.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/data.h"
#include "include/cef_v8.h"

struct mrb_cef {
   RClass* cef_module;
   RClass* v8_module;
   RClass* js_object_class;
   RClass* js_exception_class;
};

void 
mrb_cef_init(mrb_state* mrb);

mrb_value 
mrb_cef_v8_js_object_set(mrb_state* mrb, mrb_value self);

mrb_value 
mrb_cef_v8_value_wrap(mrb_state* mrb, CefRefPtr<CefV8Value> ref);

CefRefPtr<CefV8Value> 
mrb_cef_v8_value_unwrap(mrb_state* mrb, mrb_value cef_v8value);

mrb_value 
mrb_cef_v8_exception_wrap(mrb_state* mrb, CefRefPtr<CefV8Exception> ref);

CefRefPtr<CefV8Exception> 
mrb_cef_v8_exception_unwrap(mrb_state* mrb, mrb_value cef_v8value);

#endif
