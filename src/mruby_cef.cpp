#include <string>
#include "mruby.h"
#include "mruby_cef.h"
#include "mruby_v8_handler.h"

using namespace std;

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

/* Type Checks */

#define TYPE_CHECK_FN(fn, checkMethod) \
   static mrb_value                                            \
   fn (mrb_state *mrb, mrb_value self) {                       \
      if (mrb_cef_v8_value_unwrap(mrb, self)->checkMethod()){  \
         return mrb_true_value();                              \
      }                                                        \
      return mrb_false_value();                                \
   }

TYPE_CHECK_FN(mrb_cef_v8_value_is_undefined, IsUndefined)
TYPE_CHECK_FN(mrb_cef_v8_value_is_null, IsNull)
TYPE_CHECK_FN(mrb_cef_v8_value_is_bool, IsBool)
TYPE_CHECK_FN(mrb_cef_v8_value_is_int, IsInt)
TYPE_CHECK_FN(mrb_cef_v8_value_is_uint, IsUInt)
TYPE_CHECK_FN(mrb_cef_v8_value_is_double, IsDouble)
TYPE_CHECK_FN(mrb_cef_v8_value_is_date, IsDate)
TYPE_CHECK_FN(mrb_cef_v8_value_is_string, IsString)
TYPE_CHECK_FN(mrb_cef_v8_value_is_object, IsObject)
TYPE_CHECK_FN(mrb_cef_v8_value_is_array, IsArray)
TYPE_CHECK_FN(mrb_cef_v8_value_is_function, IsFunction)

#undef TYPE_CHECK_FN

/* Getting JS Values */

static mrb_value
mrb_cef_v8_value_get_bool_value(mrb_state *mrb, mrb_value self) {
   CefRefPtr<CefV8Value> js = mrb_cef_v8_value_unwrap(mrb, self);
   if (js->GetBoolValue()) {
      return mrb_true_value();
   }
   return mrb_false_value();
}

static mrb_value
mrb_cef_v8_value_get_int_value(mrb_state *mrb, mrb_value self) {
   CefRefPtr<CefV8Value> js = mrb_cef_v8_value_unwrap(mrb, self);
   mrb_value rb;
   SET_INT_VALUE(rb, js->GetIntValue());
   return rb;
}

static mrb_value
mrb_cef_v8_value_get_double_value(mrb_state *mrb, mrb_value self) {
   CefRefPtr<CefV8Value> js = mrb_cef_v8_value_unwrap(mrb, self);
   mrb_value rb;
   SET_FLOAT_VALUE(MRB, rb, js->GetDoubleValue());
   return rb;
}

static mrb_value
mrb_cef_v8_value_get_string_value(mrb_state *mrb, mrb_value self) {
   CefRefPtr<CefV8Value> js = mrb_cef_v8_value_unwrap(mrb, self);
   mrb_value rb;
   return mrb_str_new_cstr(mrb, js->GetStringValue().ToString().c_str());
}

/* Object Creation */

static mrb_value
mrb_cef_v8_create_undefined(mrb_state *mrb, mrb_value self) {
   return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateUndefined());
}

static mrb_value
mrb_cef_v8_create_null(mrb_state *mrb, mrb_value self) {
   return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateNull());
}

static mrb_value
mrb_cef_v8_create_bool(mrb_state *mrb, mrb_value self) {
   mrb_value param;
   mrb_get_args(mrb, "o", &param);
   return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateBool(mrb_test(param)));
}

static mrb_value
mrb_cef_v8_create_int(mrb_state *mrb, mrb_value self) {
   mrb_value int_param;
   mrb_get_args(mrb, "i", &int_param);
   return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateInt(int_param.value.i));
}

static mrb_value
mrb_cef_v8_create_float(mrb_state *mrb, mrb_value self) {
   mrb_value param;
   mrb_get_args(mrb, "f", &param);
   return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateDouble(mrb_float(param)));
}

static mrb_value
mrb_cef_v8_create_string(mrb_state *mrb, mrb_value self) {
   mrb_value rbString;
   mrb_get_args(mrb, "S", &rbString);
   char* cString = mrb_str_to_cstr(mrb, rbString);
   return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateString(cString));
}

static mrb_value
mrb_cef_v8_create_object(mrb_state *mrb, mrb_value self) {
   return mrb_cef_v8_value_wrap(mrb, CefV8Value::CreateObject(NULL));
}

static mrb_value
mrb_cef_v8_create_function(mrb_state *mrb, mrb_value self) {
   mrb_value name;
   mrb_value block;

   mrb_get_args(mrb, "o&", &name, &block);

   string name_string = mrb_str_to_cstr(mrb, mrb_funcall(mrb, name, "to_s", 0));

   auto handler = new MRubyV8Handler(mrb, name_string, block);
   CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(name_string, handler);
   
   // TODO: Keep a reference to the func in ruby-land so it's not garbage collected
   return mrb_cef_v8_value_wrap(mrb, func);
}

static mrb_value
mrb_cef_v8_get_window(mrb_state* mrb, mrb_value self) {
   return mrb_cef_v8_value_wrap(mrb, CefV8Context::GetCurrentContext()->GetGlobal());
}

mrb_value
mrb_cef_v8_js_object_get_property(mrb_state* mrb, mrb_value self) {
   mrb_value key_param;
   mrb_get_args(mrb, "o", &key_param);

   CefRefPtr<CefV8Value> jsThis = mrb_cef_v8_value_unwrap(mrb, self);

   if (key_param.tt == MRB_TT_FIXNUM) {
      return mrb_cef_v8_value_wrap(mrb, jsThis->GetValue(key_param.value.i));
   }
   else {
      CefString key(
         mrb_str_to_cstr(
         mrb,
         mrb_funcall(mrb, key_param, "to_s", 0)));

      return mrb_cef_v8_value_wrap(mrb, jsThis->GetValue(key));
   }
}

mrb_value
mrb_cef_v8_js_object_set_property(mrb_state* mrb, mrb_value self) {
  mrb_value key_param;
  mrb_value value_param;

  mrb_get_args(mrb, "oo", &key_param, &value_param);

  CefRefPtr<CefV8Value> jsObjPtr = mrb_cef_v8_value_unwrap(mrb, self);

  // TODO: Implicit conversions if value param is a ruby type
  CefRefPtr<CefV8Value> jsValuePtr = mrb_cef_v8_value_unwrap(mrb, value_param);

  CefString key(
    mrb_str_to_cstr(
      mrb,
      mrb_funcall(mrb, key_param, "to_s", 0)));

  jsObjPtr->SetValue(key, jsValuePtr, CefV8Value::PropertyAttribute::V8_PROPERTY_ATTRIBUTE_NONE);
  return self;
}

mrb_value
mrb_cef_v8_js_object_apply(mrb_state* mrb, mrb_value self) {
   mrb_value context;
   mrb_value* args;
   int argc;
   mrb_get_args(mrb, "oa", &context, &args, &argc);

   CefV8ValueList js_args;
   for (int i = 0; i < argc; ++i) {
      js_args.push_back(
         mrb_cef_v8_value_unwrap(mrb, *(args + i))
      );
   }

   CefRefPtr<CefV8Value> js_context = mrb_cef_v8_value_unwrap(mrb, context);
   CefRefPtr<CefV8Value> js_fn = mrb_cef_v8_value_unwrap(mrb, self);

   return mrb_cef_v8_value_wrap(mrb, js_fn->ExecuteFunction(js_context, js_args));
}

mrb_value
mrb_cef_v8_exec(mrb_state* mrb, mrb_value self) {
  mrb_value script_param;
  mrb_get_args(mrb, "S", &script_param);

  auto script_text = mrb_str_to_cstr(mrb, script_param);

  auto context = CefV8Context::GetCurrentContext();
  context->GetFrame()->ExecuteJavaScript(CefString(script_text), context->GetFrame()->GetURL(), 0);
  return mrb_nil_value();
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
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_bool", mrb_cef_v8_create_bool, MRB_ARGS_ARG(1, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_int", mrb_cef_v8_create_int, MRB_ARGS_ARG(1, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_float", mrb_cef_v8_create_float, MRB_ARGS_ARG(1, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_string", mrb_cef_v8_create_string, MRB_ARGS_ARG(1, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_object", mrb_cef_v8_create_object, MRB_ARGS_ARG(0, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "create_function", mrb_cef_v8_create_function, MRB_ARGS_ARG(2, 0));
  mrb_define_class_method(mrb, mrb_cef.v8_module, "exec", mrb_cef_v8_exec, MRB_ARGS_ARG(1, 0));

#define TYPE_CHECK_BINDING(ruby_fn) mrb_define_method(mrb, mrb_cef.js_object_class, #ruby_fn, mrb_cef_v8_value_ ## ruby_fn, MRB_ARGS_NONE());
  TYPE_CHECK_BINDING(is_undefined);
  TYPE_CHECK_BINDING(is_null);
  TYPE_CHECK_BINDING(is_bool);
  TYPE_CHECK_BINDING(is_int);
  TYPE_CHECK_BINDING(is_uint);
  TYPE_CHECK_BINDING(is_double);
  TYPE_CHECK_BINDING(is_date);
  TYPE_CHECK_BINDING(is_string);
  TYPE_CHECK_BINDING(is_object);
  TYPE_CHECK_BINDING(is_array);
  TYPE_CHECK_BINDING(is_function);
#undef TYPE_CHECK_BINDING

  mrb_define_method(mrb, mrb_cef.js_object_class, "bool_value", mrb_cef_v8_value_get_bool_value, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_cef.js_object_class, "int_value", mrb_cef_v8_value_get_int_value, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_cef.js_object_class, "double_value", mrb_cef_v8_value_get_double_value, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_cef.js_object_class, "string_value", mrb_cef_v8_value_get_string_value, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_cef.js_object_class, "[]", mrb_cef_v8_js_object_get_property, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_cef.js_object_class, "[]=", mrb_cef_v8_js_object_set_property, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, mrb_cef.js_object_class, "apply", mrb_cef_v8_js_object_apply, MRB_ARGS_REQ(2));
  
}

void mrb_mruby_cef_gem_final(mrb_state* mrb) {}

#ifdef __cplusplus
}
#endif
