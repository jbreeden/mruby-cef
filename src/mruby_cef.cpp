#include "mruby.h"
#include "mruby_cef.h"

struct mrb_cef mrb_cef;

static void free_cef_ref_ptr(mrb_state* mrb, void* ptr) {
   free(ptr);
};

static const mrb_data_type cef_v8value_data_type = {
   "cef_v8value_t", free_cef_ref_ptr
};

mrb_value mrb_cef_v8_value_wrap(mrb_state* mrb, CefRefPtr<CefV8Value> ref) {
  CefRefPtr<CefV8Value>* ptr = new CefRefPtr<CefV8Value>();
  *ptr = ref;
  RData* data = mrb_data_object_alloc(mrb, mrb->object_class, ptr, &cef_v8value_data_type);
  mrb_value ruby_value = mrb_funcall(mrb, mrb_obj_value(mrb_cef.js_object_class), "new", 0);
  mrb_iv_set(mrb, ruby_value, mrb_intern_cstr(mrb, "value"), mrb_obj_value(data));
  return ruby_value;
}

CefRefPtr<CefV8Value>
mrb_cef_v8_value_unwrap(mrb_state* mrb, mrb_value cef_v8value){
  mrb_value wrapped_value = mrb_iv_get(mrb, cef_v8value, mrb_intern_cstr(mrb, "value"));
  return *(CefRefPtr<CefV8Value>*) DATA_PTR(wrapped_value);
}

static mrb_value
mrb_cef_v8_create_undefined(mrb_state *mrb, mrb_value self) {
  return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateUndefined());
}

static mrb_value
mrb_cef_v8_create_null(mrb_state *mrb, mrb_value self) {
  return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateNull());
}

static mrb_value
mrb_cef_v8_create_int(mrb_state *mrb, mrb_value self) {
  mrb_value int_param;
  mrb_get_args(mrb, "i", &int_param);
  return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateInt(int_param.value.i));
}

static mrb_value
mrb_cef_v8_create_string(mrb_state *mrb, mrb_value self) {
  mrb_value rbString;
  mrb_get_args(mrb, "S", &rbString);
  char* cString = mrb_str_to_cstr(mrb, rbString);

  return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateString(cString));
}

static mrb_value
mrb_cef_v8_get_window(mrb_state* mrb, mrb_value self) {
  return mrb_cef_v8_value_wrap(mrb, CefV8Context::GetCurrentContext()->GetGlobal());
}

mrb_value
mrb_cef_v8_js_object_set(mrb_state* mrb, mrb_value self) {
  mrb_value key_param;
  mrb_value value_param;

  mrb_get_args(mrb, "oo", &key_param, &value_param);

  CefRefPtr<CefV8Value> jsObjPtr = mrb_cef_v8_value_unwrap(mrb, self);

  CefRefPtr<CefV8Value> jsValuePtr = mrb_cef_v8_value_unwrap(mrb, value_param);

  CefString key(
    mrb_str_to_cstr(
      mrb,
      mrb_funcall(mrb, key_param, "to_s", 0)));

  jsObjPtr->SetValue(key, jsValuePtr, CefV8Value::PropertyAttribute::V8_PROPERTY_ATTRIBUTE_NONE);
  return self;
}

mrb_value
mrb_cef_v8_exec(mrb_state* mrb, mrb_value self) {
  mrb_value script_param;
  mrb_get_args(mrb, "S", &script_param);

  auto script_text = mrb_str_to_cstr(mrb, script_param);

  auto context = CefV8Context::GetCurrentContext();
  context->GetFrame()->ExecuteJavaScript(CefString(script_text), context->GetFrame()->GetURL(), 0);
  return self;
}

#ifdef __cplusplus
extern "C" {
#endif

void mrb_mruby_cef_gem_init(mrb_state* mrb) {
  mrb_cef.cef_module = mrb_define_module(mrb, "Cef");
  mrb_cef.v8_module = mrb_define_module_under(mrb, mrb_cef.cef_module, "V8");
  mrb_cef.js_object_class = mrb_define_class_under(mrb, mrb_cef.v8_module, "JsObject", mrb->object_class);

  mrb_define_class_method(mrb, mrb_cef.v8_module, "window", mrb_cef_v8_get_window, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_undefined", mrb_cef_v8_create_undefined, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_null", mrb_cef_v8_create_null, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_string", mrb_cef_v8_create_string, MRB_ARGS_ARG(1, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_int", mrb_cef_v8_create_int, MRB_ARGS_ARG(1, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "exec", mrb_cef_v8_exec, MRB_ARGS_ARG(1, 0));

  mrb_define_method(mrb, mrb_cef.js_object_class, "[]=", mrb_cef_v8_js_object_set, MRB_ARGS_REQ(2));
}

void mrb_mruby_cef_gem_final(mrb_state* mrb) {}

#ifdef __cplusplus
}
#endif
