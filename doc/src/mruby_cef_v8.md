Module Cef::V8
==============

### `::create_undefined`
- Creates a JsObject containing a JavaScript undefined

### `::create_null`
- Creates a JsObject containing a JavaScript null

### `::create_bool(value)`
- Creates a JsObject containing a JavaScript bool
- Args
  + `value`: If `nil` or `flase`, returns a JavaScript fale, else returns a JavaScript true

### `::create_int(value)`
- Creates a JsObject containing a JavaScript int
- Args
  + `value`: A number to convert to a JavaScript int

### `::create_float(value)`
- Creates a JsObject containing a JavaScript float
- Args
  + `value`: A number to convert to a JavaScript float

### `::create_string(rb_string)`
- Creates a JsObject containing a JavaScript string
- Args
  + `rb_string`, the ruby string to convert to a JavaScript value

### `::create_object`
- Creates a JsObject containing a JavaScript object

### `::create_function(name, &block)`
- Creates a JsObject containing a JavaScript function
- Args
  + `name`: The name of the function
  + `block`: A block accepting an array of JsObjects as the only parameter. <br/>
             This block will be executed when the funciton is called from JavaScript. <br/>
             If the block returns a JsObject, it will be unwrapped and returned to the caller. <br/>
             If the block returns anything else, `undefined` will be returned to JavaScript. <br/>
             If the block throws, a JavaScript Error object will be thrown containing the result <br/>
             of calling `to_s` on the ruby exception. <br/>

### `::window`
- Returns the global `window` object fromt the current V8 context

### `::eval(str)`
- Evaluates the given string as JavaScript in the current V8 context

class Cef::V8::JsObject
=======================

### `#bool_value`
- Returns the bool value of this JsObject. (Should check type with `is_bool?` first)

### `#int_value`
- Returns the int value of this JsObject. (Should check type with `is_int?` first)

### `#double_value`
- Returns the double value of this JsObject. (Should check type with `is_double?` first)

### `#string_value`
- Returns the string value of this JsObject. (Should check type with `is_string?` first)

### `#is_undefined?`
- Returns true if this JsObject is a JavaScript undefined, else false

### `#is_null?`
- Returns true if this JsObject is a JavaScript null, else false

### `#is_bool?`
- Returns true if this JsObject is a JavaScript bool, else false

### `#is_int?`
- Returns true if this JsObject is a JavaScript int, else false

### `#is_uint?`
- Returns true if this JsObject is a JavaScript uint, else false

### `#is_double?`
- Returns true if this JsObject is a JavaScript double, else false

### `#is_date?`
- Returns true if this JsObject is a JavaScript date, else false

### `#is_string?`
- Returns true if this JsObject is a JavaScript string, else false

### `#is_object?`
- Returns true if this JsObject is a JavaScript object, else false

### `#is_array?`
- Returns true if this JsObject is a JavaScript array, else false

### `#is_function?`
- Returns true if this JsObject is a JavaScript function, else false

### `#[](key)`
- Returns the value of the property named by `key` on the underlying JavaScript object

### `#[](key, value)`
- Sets the value of the property named by `key` on the underlying JavaScript object to `value`
  (which must be a JsObject)

### `#apply(context, args, &exception_handler)`
- Used to call a JavaScript function
- Pretty much just like `apply` from javascript
- Args
  + `context`: What to bind `this` to in the invoked function
  + `args`: An array of JsObject values to supply as parameters to the function
  + `exception_handler`: A block accepting a single parameter. <br/>
                         If the JavaScript function throws, the exception is wrapped
                         in a Cef::V8::JsException and pass to this block.
- Return
  + The return value of the JavaScript function, wrapped in a `Cef::V8::JsObject`
- Notes
  + If the JavaScript function throws, you must either handle the exception or
    bail out immediately.
  + If you do not supply the `exception_handler` param, any thrown exceptions are
    immediately raised in JavaScript land. Continuing to execute code that interacts
    with the JavaScript context is an error (semantically speaking). For this reason,
    if there is any chance an exception will be thrown by the called function, you
    should always provid an `exception_handler` block.
  + If you can't handler the `exception` passed to the `exception_handler` block,
    simply `raise exception.message` to re-throw it in JavaScript land.

# Class Cef::V8::JsException

### `#message`
- Returns the message from the wrapped JavaScript exception as a string

