// Copyright 2007-2009 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * This test file is an adaptation of test-api.cc from the v8 source tree.  We
 * would need to import a whole bunch of their source files to just drop in that
 * test file, which we don't really want to do.
 */

#include "v8api_test_harness.h"
#include "v8api_test_checks.h"

typedef JSUint16 uint16_t;
typedef JSInt32 int32_t;
typedef JSUint32 uint32_t;
typedef JSInt64 int64_t;

////////////////////////////////////////////////////////////////////////////////
//// Helpers

static inline v8::Local<v8::Number> v8_num(double x) {
  return v8::Number::New(x);
}
static inline v8::Local<v8::String> v8_str(const char* x) {
  return v8::String::New(x);
}

static inline v8::Local<v8::Script> v8_compile(const char* x) {
  return v8::Script::Compile(v8_str(x));
}

// Helper function that compiles and runs the source.
static inline v8::Local<v8::Value> CompileRun(const char* source) {
  return v8::Script::Compile(v8::String::New(source))->Run();
}

static void ExpectString(const char* code, const char* expected) {
  Local<Value> result = CompileRun(code);
  CHECK(result->IsString());
  String::AsciiValue ascii(result);
  CHECK_EQ(expected, *ascii);
}

static void ExpectBoolean(const char* code, bool expected) {
  Local<Value> result = CompileRun(code);
  CHECK(result->IsBoolean());
  CHECK_EQ(expected, result->BooleanValue());
}

static void ExpectTrue(const char* code) {
  ExpectBoolean(code, true);
}

static void ExpectFalse(const char* code) {
  ExpectBoolean(code, false);
}

static void ExpectObject(const char* code, Local<Value> expected) {
  Local<Value> result = CompileRun(code);
  CHECK(result->Equals(expected));
}

void CheckProperties(v8::Handle<v8::Value> val, int elmc, const char* elmv[]) {
  v8::Handle<v8::Object> obj = val.As<v8::Object>();
  v8::Handle<v8::Array> props = obj->GetPropertyNames();
  CHECK_EQ(elmc, props->Length());
  for (int i = 0; i < elmc; i++) {
    v8::String::Utf8Value elm(props->Get(v8::Integer::New(i)));
    CHECK_EQ(elmv[i], *elm);
  }
}

static void ExpectUndefined(const char* code) {
  Local<Value> result = CompileRun(code);
  CHECK(result->IsUndefined());
}

static int StrCmp16(uint16_t* a, uint16_t* b) {
  while (true) {
    if (*a == 0 && *b == 0) return 0;
    if (*a != *b) return 0 + *a - *b;
    a++;
    b++;
  }
}

static int StrNCmp16(uint16_t* a, uint16_t* b, int n) {
  while (true) {
    if (n-- == 0) return 0;
    if (*a == 0 && *b == 0) return 0;
    if (*a != *b) return 0 + *a - *b;
    a++;
    b++;
  }
}

static v8::Handle<Value> GetFlabby(const v8::Arguments& args) {
  // XXX Calls to ApiTestFuzzer::Fuzz have been commented out since
  //     we aren't currently doing any thread fuzzing.
  //ApiTestFuzzer::Fuzz();
  return v8_num(17.2);
}

static v8::Handle<Value> GetKnurd(Local<String> property, const AccessorInfo&) {
  //ApiTestFuzzer::Fuzz();
  return v8_num(15.2);
}

v8::Persistent<Value> xValue;

static void SetXValue(Local<String> name,
                      Local<Value> value,
                      const AccessorInfo& info) {
  CHECK_EQ(value, v8_num(4));
  CHECK_EQ(info.Data(), v8_str("donut"));
  CHECK_EQ(name, v8_str("x"));
  CHECK(xValue.IsEmpty());
  xValue = v8::Persistent<Value>::New(value);
}

static v8::Handle<Value> GetXValue(Local<String> name,
                                   const AccessorInfo& info) {
  //ApiTestFuzzer::Fuzz();
  CHECK_EQ(info.Data(), v8_str("donut"));
  CHECK_EQ(name, v8_str("x"));
  return name;
}

static int signature_callback_count;

static v8::Handle<Value> IncrementingSignatureCallback(
    const v8::Arguments& args) {
  //ApiTestFuzzer::Fuzz();
  signature_callback_count++;
  v8::Handle<v8::Array> result = v8::Array::New(args.Length());
  for (int i = 0; i < args.Length(); i++)
    result->Set(v8::Integer::New(i), args[i]);
  return result;
}

static v8::Handle<Value> SignatureCallback(const v8::Arguments& args) {
  //ApiTestFuzzer::Fuzz();
  v8::Handle<v8::Array> result = v8::Array::New(args.Length());
  for (int i = 0; i < args.Length(); i++) {
    result->Set(v8::Integer::New(i), args[i]);
  }
  return result;
}

// A LocalContext holds a reference to a v8::Context.
class LocalContext {
 public:
  // TODO use v8::ExtensionConfiguration again
  LocalContext(//v8::ExtensionConfiguration* extensions = 0,
               v8::Handle<v8::ObjectTemplate> global_template =
                   v8::Handle<v8::ObjectTemplate>(),
               v8::Handle<v8::Value> global_object = v8::Handle<v8::Value>())
    : context_(v8::Context::New()) {
//    : context_(v8::Context::New(extensions, global_template, global_object)) {
    context_->Enter();
  }

  virtual ~LocalContext() {
    context_->Exit();
    context_.Dispose();
  }

  v8::Context* operator->() { return *context_; }
  v8::Context* operator*() { return *context_; }
  bool IsReady() { return !context_.IsEmpty(); }

  v8::Local<v8::Context> local() {
    return v8::Local<v8::Context>::New(context_);
  }

 private:
  v8::Persistent<v8::Context> context_;
};

namespace v8 {
namespace internal {
template <typename T>
static T* NewArray(int size) {
  T* result = new T[size];
  do_check_true(result);
  return result;
}


template <typename T>
static void DeleteArray(T* array) {
  delete[] array;
}
} // namespace internal
} // namespace v8

////////////////////////////////////////////////////////////////////////////////
//// Tests

// from test-api.cc:125
void
test_Handles()
{
  v8::HandleScope scope;
  Local<Context> local_env;
  {
    LocalContext env;
    local_env = env.local();
  }

  // Local context should still be live.
  CHECK(!local_env.IsEmpty());
  local_env->Enter();

  v8::Handle<v8::Primitive> undef = v8::Undefined();
  CHECK(!undef.IsEmpty());
  CHECK(undef->IsUndefined());

  const char* c_source = "1 + 2 + 3";
  Local<String> source = String::New(c_source);
  Local<Script> script = Script::Compile(source);
  CHECK_EQ(6, script->Run()->Int32Value());

  local_env->Exit();
}

// from test-api.cc:150
void
test_ReceiverSignature()
{ }

// from test-api.cc:196
void
test_ArgumentSignature()
{ }

// from test-api.cc:258
void
test_HulIgennem()
{
  v8::HandleScope scope;
  LocalContext env;
  v8::Handle<v8::Primitive> undef = v8::Undefined();
  Local<String> undef_str = undef->ToString();
  char* value = i::NewArray<char>(undef_str->Length() + 1);
  undef_str->WriteAscii(value);
  CHECK_EQ(0, strcmp(value, "undefined"));
  i::DeleteArray(value);
}

// from test-api.cc:270
void
test_Access()
{
  v8::HandleScope scope;
  LocalContext env;
  Local<v8::Object> obj = v8::Object::New();
  Local<Value> foo_before = obj->Get(v8_str("foo"));
  CHECK(foo_before->IsUndefined());
  Local<String> bar_str = v8_str("bar");
  obj->Set(v8_str("foo"), bar_str);
  Local<Value> foo_after = obj->Get(v8_str("foo"));
  CHECK(!foo_after->IsUndefined());
  CHECK(foo_after->IsString());
  CHECK_EQ(bar_str, foo_after);
}

// from test-api.cc:285
void
test_AccessElement()
{
  v8::HandleScope scope;
  LocalContext env;
  Local<v8::Object> obj = v8::Object::New();
  Local<Value> before = obj->Get(1);
  CHECK(before->IsUndefined());
  Local<String> bar_str = v8_str("bar");
  obj->Set(1, bar_str);
  Local<Value> after = obj->Get(1);
  CHECK(!after->IsUndefined());
  CHECK(after->IsString());
  CHECK_EQ(bar_str, after);

  Local<v8::Array> value = CompileRun("[\"a\", \"b\"]").As<v8::Array>();
  CHECK_EQ(v8_str("a"), value->Get(0));
  CHECK_EQ(v8_str("b"), value->Get(1));
}

// from test-api.cc:304
void
test_Script()
{
  v8::HandleScope scope;
  LocalContext env;
  const char* c_source = "1 + 2 + 3";
  Local<String> source = String::New(c_source);
  Local<Script> script = Script::Compile(source);
  CHECK_EQ(6, script->Run()->Int32Value());
}

// from test-api.cc:381
void
test_ScriptUsingStringResource()
{ }

// from test-api.cc:406
void
test_ScriptUsingAsciiStringResource()
{ }

// from test-api.cc:427
void
test_ScriptMakingExternalString()
{ }

// from test-api.cc:452
void
test_ScriptMakingExternalAsciiString()
{ }

// from test-api.cc:478
void
test_MakingExternalStringConditions()
{ }

// from test-api.cc:524
void
test_MakingExternalAsciiStringConditions()
{ }

// from test-api.cc:561
void
test_UsingExternalString()
{ }

// from test-api.cc:579
void
test_UsingExternalAsciiString()
{ }

// from test-api.cc:597
void
test_ScavengeExternalString()
{ }

// from test-api.cc:616
void
test_ScavengeExternalAsciiString()
{ }

// from test-api.cc:655
void
test_ExternalStringWithDisposeHandling()
{ }

// from test-api.cc:701
void
test_StringConcat()
{ }

// from test-api.cc:747
void
test_GlobalProperties()
{
  v8::HandleScope scope;
  LocalContext env;
  v8::Handle<v8::Object> global = env->Global();
  global->Set(v8_str("pi"), v8_num(3.1415926));
  Local<Value> pi = global->Get(v8_str("pi"));
  CHECK_EQ(3.1415926, pi->NumberValue());
}

// from test-api.cc:776
void
test_FunctionTemplate()
{ }

// from test-api.cc:844
void
test_ExternalWrap()
{ }

// from test-api.cc:890
void
test_FindInstanceInPrototypeChain()
{ }

// from test-api.cc:937
void
test_TinyInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  int32_t value = 239;
  Local<v8::Integer> value_obj = v8::Integer::New(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

// from test-api.cc:946
void
test_BigSmiInteger()
{ }

// from test-api.cc:960
void
test_BigInteger()
{ }

// from test-api.cc:977
void
test_TinyUnsignedInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  uint32_t value = 239;
  Local<v8::Integer> value_obj = v8::Integer::NewFromUnsigned(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

// from test-api.cc:986
void
test_BigUnsignedSmiInteger()
{ }

// from test-api.cc:997
void
test_BigUnsignedInteger()
{ }

// from test-api.cc:1008
void
test_OutOfSignedRangeUnsignedInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  uint32_t INT32_MAX_AS_UINT = (1U << 31) - 1;
  uint32_t value = INT32_MAX_AS_UINT + 1;
  CHECK(value > INT32_MAX_AS_UINT);  // No overflow.
  Local<v8::Integer> value_obj = v8::Integer::NewFromUnsigned(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

// from test-api.cc:1019
void
test_Number()
{
  v8::HandleScope scope;
  LocalContext env;
  double PI = 3.1415926;
  Local<v8::Number> pi_obj = v8::Number::New(PI);
  CHECK_EQ(PI, pi_obj->NumberValue());
}

// from test-api.cc:1028
void
test_ToNumber()
{
  v8::HandleScope scope;
  LocalContext env;
  Local<String> str = v8_str("3.1415926");
  CHECK_EQ(3.1415926, str->NumberValue());
  v8::Handle<v8::Boolean> t = v8::True();
  CHECK_EQ(1.0, t->NumberValue());
  v8::Handle<v8::Boolean> f = v8::False();
  CHECK_EQ(0.0, f->NumberValue());
}

// from test-api.cc:1040
void
test_Date()
{
  v8::HandleScope scope;
  LocalContext env;
  double PI = 3.1415926;
  Local<Value> date_obj = v8::Date::New(PI);
  CHECK(date_obj->IsDate());
  CHECK_EQ(3.0, date_obj->NumberValue());
}

// from test-api.cc:1049
void
test_Boolean()
{
  v8::HandleScope scope;
  LocalContext env;
  v8::Handle<v8::Boolean> t = v8::True();
  CHECK(t->Value());
  v8::Handle<v8::Boolean> f = v8::False();
  CHECK(!f->Value());
  v8::Handle<v8::Primitive> u = v8::Undefined();
  CHECK(!u->BooleanValue());
  v8::Handle<v8::Primitive> n = v8::Null();
  CHECK(!n->BooleanValue());
  v8::Handle<String> str1 = v8_str("");
  CHECK(!str1->BooleanValue());
  v8::Handle<String> str2 = v8_str("x");
  CHECK(str2->BooleanValue());
  CHECK(!v8::Number::New(0)->BooleanValue());
  CHECK(v8::Number::New(-1)->BooleanValue());
  CHECK(v8::Number::New(1)->BooleanValue());
  CHECK(v8::Number::New(42)->BooleanValue());
  CHECK(!v8_compile("NaN")->Run()->BooleanValue());
}

// from test-api.cc:1084
void
test_GlobalPrototype()
{ }

// from test-api.cc:1103
void
test_ObjectTemplate()
{ }

// from test-api.cc:1139
void
test_DescriptorInheritance()
{ }

// from test-api.cc:1207
void
test_NamedPropertyHandlerGetter()
{ }

// from test-api.cc:1243
void
test_IndexedPropertyHandlerGetter()
{ }

// from test-api.cc:1344
void
test_PropertyHandlerInPrototype()
{ }

// from test-api.cc:1413
void
test_PrePropertyHandler()
{ }

// from test-api.cc:1431
void
test_UndefinedIsNotEnumerable()
{ }

// from test-api.cc:1468
void
test_DeepCrossLanguageRecursion()
{ }

// from test-api.cc:1502
void
test_CallbackExceptionRegression()
{ }

// from test-api.cc:1518
void
test_FunctionPrototype()
{ }

// from test-api.cc:1529
void
test_InternalFields()
{ }

// from test-api.cc:1544
void
test_GlobalObjectInternalFields()
{ }

// from test-api.cc:1558
void
test_InternalFieldsNativePointers()
{ }

// from test-api.cc:1590
void
test_InternalFieldsNativePointersAndExternal()
{ }

// from test-api.cc:1628
void
test_IdentityHash()
{ }

// from test-api.cc:1672
void
test_HiddenProperties()
{ }

// from test-api.cc:1728
void
test_HiddenPropertiesWithInterceptors()
{ }

// from test-api.cc:1748
void
test_External()
{ }

// from test-api.cc:1780
void
test_GlobalHandle()
{
  v8::Persistent<String> global;
  {
    v8::HandleScope scope;
    Local<String> str = v8_str("str");
    global = v8::Persistent<String>::New(str);
  }
  CHECK_EQ(global->Length(), 3);
  global.Dispose();
}

// from test-api.cc:1792
void
test_ScriptException()
{
  v8::HandleScope scope;
  LocalContext env;
  Local<Script> script = Script::Compile(v8_str("throw 'panama!';"));
  v8::TryCatch try_catch;
  Local<Value> result = script->Run();
  CHECK(result.IsEmpty());
  CHECK(try_catch.HasCaught());
  String::AsciiValue exception_value(try_catch.Exception());
  //CHECK_EQ(*exception_value, "panama!");
  do_check_true(0 == strcmp(*exception_value, "panama!"));
}

// from test-api.cc:1817
void
test_MessageHandlerData()
{ }

// from test-api.cc:1835
void
test_GetSetProperty()
{
  v8::HandleScope scope;
  LocalContext context;
  context->Global()->Set(v8_str("foo"), v8_num(14));
  context->Global()->Set(v8_str("12"), v8_num(92));
  context->Global()->Set(v8::Integer::New(16), v8_num(32));
  context->Global()->Set(v8_num(13), v8_num(56));
  Local<Value> foo = Script::Compile(v8_str("this.foo"))->Run();
  CHECK_EQ(14, foo->Int32Value());
  Local<Value> twelve = Script::Compile(v8_str("this[12]"))->Run();
  CHECK_EQ(92, twelve->Int32Value());
  Local<Value> sixteen = Script::Compile(v8_str("this[16]"))->Run();
  CHECK_EQ(32, sixteen->Int32Value());
  Local<Value> thirteen = Script::Compile(v8_str("this[13]"))->Run();
  CHECK_EQ(56, thirteen->Int32Value());
  CHECK_EQ(92, context->Global()->Get(v8::Integer::New(12))->Int32Value());
  CHECK_EQ(92, context->Global()->Get(v8_str("12"))->Int32Value());
  CHECK_EQ(92, context->Global()->Get(v8_num(12))->Int32Value());
  CHECK_EQ(32, context->Global()->Get(v8::Integer::New(16))->Int32Value());
  CHECK_EQ(32, context->Global()->Get(v8_str("16"))->Int32Value());
  CHECK_EQ(32, context->Global()->Get(v8_num(16))->Int32Value());
  CHECK_EQ(56, context->Global()->Get(v8::Integer::New(13))->Int32Value());
  CHECK_EQ(56, context->Global()->Get(v8_str("13"))->Int32Value());
  CHECK_EQ(56, context->Global()->Get(v8_num(13))->Int32Value());
}

// from test-api.cc:1862
void
test_PropertyAttributes()
{
  v8::HandleScope scope;
  LocalContext context;
  // read-only
  Local<String> prop = v8_str("read_only");
  context->Global()->Set(prop, v8_num(7), v8::ReadOnly);
  CHECK_EQ(7, context->Global()->Get(prop)->Int32Value());
  Script::Compile(v8_str("read_only = 9"))->Run();
  CHECK_EQ(7, context->Global()->Get(prop)->Int32Value());
  context->Global()->Set(prop, v8_num(10));
  CHECK_EQ(7, context->Global()->Get(prop)->Int32Value());
  // dont-delete
  prop = v8_str("dont_delete");
  context->Global()->Set(prop, v8_num(13), v8::DontDelete);
  CHECK_EQ(13, context->Global()->Get(prop)->Int32Value());
  Script::Compile(v8_str("delete dont_delete"))->Run();
  CHECK_EQ(13, context->Global()->Get(prop)->Int32Value());
}

// from test-api.cc:1882
void
test_Array()
{
  v8::HandleScope scope;
  LocalContext context;
  Local<v8::Array> array = v8::Array::New();
  CHECK_EQ(0, array->Length());
  CHECK(array->Get(0)->IsUndefined());
  CHECK(!array->Has(0));
  CHECK(array->Get(100)->IsUndefined());
  CHECK(!array->Has(100));
  array->Set(2, v8_num(7));
  CHECK_EQ(3, array->Length());
  CHECK(!array->Has(0));
  CHECK(!array->Has(1));
  CHECK(array->Has(2));
  CHECK_EQ(7, array->Get(2)->Int32Value());
  Local<Value> obj = Script::Compile(v8_str("[1, 2, 3]"))->Run();
  Local<v8::Array> arr = obj.As<v8::Array>();
  CHECK_EQ(3, arr->Length());
  CHECK_EQ(1, arr->Get(0)->Int32Value());
  CHECK_EQ(2, arr->Get(1)->Int32Value());
  CHECK_EQ(3, arr->Get(2)->Int32Value());
}

// from test-api.cc:1916
void
test_Vector()
{ }

// from test-api.cc:1954
void
test_FunctionCall()
{
  v8::HandleScope scope;
  LocalContext context;
  CompileRun(
    "function Foo() {"
    "  var result = [];"
    "  for (var i = 0; i < arguments.length; i++) {"
    "    result.push(arguments[i]);"
    "  }"
    "  return result;"
    "}");
  Local<Function> Foo =
      Local<Function>::Cast(context->Global()->Get(v8_str("Foo")));

  v8::Handle<Value>* args0 = NULL;
  Local<v8::Array> a0 = Local<v8::Array>::Cast(Foo->Call(Foo, 0, args0));
  CHECK_EQ(0, a0->Length());

  v8::Handle<Value> args1[] = { v8_num(1.1) };
  Local<v8::Array> a1 = Local<v8::Array>::Cast(Foo->Call(Foo, 1, args1));
  CHECK_EQ(1, a1->Length());
  CHECK_EQ(1.1, a1->Get(v8::Integer::New(0))->NumberValue());

  v8::Handle<Value> args2[] = { v8_num(2.2),
                                v8_num(3.3) };
  Local<v8::Array> a2 = Local<v8::Array>::Cast(Foo->Call(Foo, 2, args2));
  CHECK_EQ(2, a2->Length());
  CHECK_EQ(2.2, a2->Get(v8::Integer::New(0))->NumberValue());
  CHECK_EQ(3.3, a2->Get(v8::Integer::New(1))->NumberValue());

  v8::Handle<Value> args3[] = { v8_num(4.4),
                                v8_num(5.5),
                                v8_num(6.6) };
  Local<v8::Array> a3 = Local<v8::Array>::Cast(Foo->Call(Foo, 3, args3));
  CHECK_EQ(3, a3->Length());
  CHECK_EQ(4.4, a3->Get(v8::Integer::New(0))->NumberValue());
  CHECK_EQ(5.5, a3->Get(v8::Integer::New(1))->NumberValue());
  CHECK_EQ(6.6, a3->Get(v8::Integer::New(2))->NumberValue());

  v8::Handle<Value> args4[] = { v8_num(7.7),
                                v8_num(8.8),
                                v8_num(9.9),
                                v8_num(10.11) };
  Local<v8::Array> a4 = Local<v8::Array>::Cast(Foo->Call(Foo, 4, args4));
  CHECK_EQ(4, a4->Length());
  CHECK_EQ(7.7, a4->Get(v8::Integer::New(0))->NumberValue());
  CHECK_EQ(8.8, a4->Get(v8::Integer::New(1))->NumberValue());
  CHECK_EQ(9.9, a4->Get(v8::Integer::New(2))->NumberValue());
  CHECK_EQ(10.11, a4->Get(v8::Integer::New(3))->NumberValue());
}

// from test-api.cc:2012
void
test_OutOfMemory()
{ }

// from test-api.cc:2053
void
test_OutOfMemoryNested()
{ }

// from test-api.cc:2082
void
test_HugeConsStringOutOfMemory()
{ }

// from test-api.cc:2108
void
test_ConstructCall()
{ }

// from test-api.cc:2168
void
test_ConversionNumber()
{
  v8::HandleScope scope;
  LocalContext env;
  // Very large number.
  CompileRun("var obj = Math.pow(2,32) * 1237;");
  Local<Value> obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(5312874545152.0, obj->ToNumber()->Value());
  CHECK_EQ(0, obj->ToInt32()->Value());
  CHECK(0u == obj->ToUint32()->Value());  // NOLINT - no CHECK_EQ for unsigned.
  // Large number.
  CompileRun("var obj = -1234567890123;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(-1234567890123.0, obj->ToNumber()->Value());
  CHECK_EQ(-1912276171, obj->ToInt32()->Value());
  CHECK(2382691125u == obj->ToUint32()->Value());  // NOLINT
  // Small positive integer.
  CompileRun("var obj = 42;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(42.0, obj->ToNumber()->Value());
  CHECK_EQ(42, obj->ToInt32()->Value());
  CHECK(42u == obj->ToUint32()->Value());  // NOLINT
  // Negative integer.
  CompileRun("var obj = -37;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(-37.0, obj->ToNumber()->Value());
  CHECK_EQ(-37, obj->ToInt32()->Value());
  CHECK(4294967259u == obj->ToUint32()->Value());  // NOLINT
  // Positive non-int32 integer.
  CompileRun("var obj = 0x81234567;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(2166572391.0, obj->ToNumber()->Value());
  CHECK_EQ(-2128394905, obj->ToInt32()->Value());
  CHECK(2166572391u == obj->ToUint32()->Value());  // NOLINT
  // Fraction.
  CompileRun("var obj = 42.3;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(42.3, obj->ToNumber()->Value());
  CHECK_EQ(42, obj->ToInt32()->Value());
  CHECK(42u == obj->ToUint32()->Value());  // NOLINT
  // Large negative fraction.
  CompileRun("var obj = -5726623061.75;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(-5726623061.75, obj->ToNumber()->Value());
  CHECK_EQ(-1431655765, obj->ToInt32()->Value());
  CHECK(2863311531u == obj->ToUint32()->Value());  // NOLINT
}

// from test-api.cc:2216
void
test_isNumberType()
{
  v8::HandleScope scope;
  LocalContext env;
  // Very large number.
  CompileRun("var obj = Math.pow(2,32) * 1237;");
  Local<Value> obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Large negative number.
  CompileRun("var obj = -1234567890123;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Small positive integer.
  CompileRun("var obj = 42;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(obj->IsInt32());
  CHECK(obj->IsUint32());
  // Negative integer.
  CompileRun("var obj = -37;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Positive non-int32 integer.
  CompileRun("var obj = 0x81234567;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(obj->IsUint32());
  // Fraction.
  CompileRun("var obj = 42.3;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Large negative fraction.
  CompileRun("var obj = -5726623061.75;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
}

// from test-api.cc:2257
void
test_ConversionException()
{ }

// from test-api.cc:2327
void
test_APICatch()
{ }

// from test-api.cc:2345
void
test_APIThrowTryCatch()
{ }

// from test-api.cc:2364
void
test_TryCatchInTryFinally()
{ }

// from test-api.cc:2399
void
test_APIThrowMessageOverwrittenToString()
{ }

// from test-api.cc:2444
void
test_APIThrowMessage()
{ }

// from test-api.cc:2458
void
test_APIThrowMessageAndVerboseTryCatch()
{ }

// from test-api.cc:2476
void
test_ExternalScriptException()
{ }

// from test-api.cc:2543
void
test_EvalInTryFinally()
{
  v8::HandleScope scope;
  LocalContext context;
  v8::TryCatch try_catch;
  CompileRun("(function() {"
             "  try {"
             "    eval('asldkf (*&^&*^');"
             "  } finally {"
             "    return;"
             "  }"
             "})()");
  CHECK(!try_catch.HasCaught());
}

// from test-api.cc:2578
void
test_ExceptionOrder()
{ }

// from test-api.cc:2642
void
test_ThrowValues()
{ }

// from test-api.cc:2668
void
test_CatchZero()
{
  v8::HandleScope scope;
  LocalContext context;
  v8::TryCatch try_catch;
  CHECK(!try_catch.HasCaught());
  Script::Compile(v8_str("throw 10"))->Run();
  CHECK(try_catch.HasCaught());
  CHECK_EQ(10, try_catch.Exception()->Int32Value());
  try_catch.Reset();
  CHECK(!try_catch.HasCaught());
  Script::Compile(v8_str("throw 0"))->Run();
  CHECK(try_catch.HasCaught());
  CHECK_EQ(0, try_catch.Exception()->Int32Value());
}

// from test-api.cc:2684
void
test_CatchExceptionFromWith()
{
  v8::HandleScope scope;
  LocalContext context;
  v8::TryCatch try_catch;
  CHECK(!try_catch.HasCaught());
  Script::Compile(v8_str("var o = {}; with (o) { throw 42; }"))->Run();
  CHECK(try_catch.HasCaught());
}

// from test-api.cc:2694
void
test_TryCatchAndFinallyHidingException()
{
  v8::HandleScope scope;
  LocalContext context;
  v8::TryCatch try_catch;
  CHECK(!try_catch.HasCaught());
  CompileRun("function f(k) { try { this[k]; } finally { return 0; } };");
  CompileRun("f({toString: function() { throw 42; }});");
  CHECK(!try_catch.HasCaught());
}

// from test-api.cc:2711
void
test_TryCatchAndFinally()
{ }

// from test-api.cc:2729
void
test_Equality()
{ }

// from test-api.cc:2761
void
test_MultiRun()
{ }

// from test-api.cc:2779
void
test_SimplePropertyRead()
{
  v8::HandleScope scope;
  Local<ObjectTemplate> templ = ObjectTemplate::New();
  templ->SetAccessor(v8_str("x"), GetXValue, NULL, v8_str("donut"));
  LocalContext context;
  context->Global()->Set(v8_str("obj"), templ->NewInstance());
  Local<Script> script = Script::Compile(v8_str("obj.x"));
  for (int i = 0; i < 10; i++) {
    Local<Value> result = script->Run();
    CHECK_EQ(result, v8_str("x"));
  }
}

// from test-api.cc:2792
void
test_DefinePropertyOnAPIAccessor()
{ }

// from test-api.cc:2840
void
test_DefinePropertyOnDefineGetterSetter()
{ }

// from test-api.cc:2893
void
test_DefineAPIAccessorOnObject()
{ }

// from test-api.cc:2967
void
test_DontDeleteAPIAccessorsCannotBeOverriden()
{ }

// from test-api.cc:3025
void
test_ElementAPIAccessor()
{ }

// from test-api.cc:3063
void
test_SimplePropertyWrite()
{ }

// from test-api.cc:3088
void
test_NamedInterceptorPropertyRead()
{ }

// from test-api.cc:3102
void
test_NamedInterceptorDictionaryIC()
{ }

// from test-api.cc:3132
void
test_NamedInterceptorDictionaryICMultipleContext()
{ }

// from test-api.cc:3186
void
test_NamedInterceptorMapTransitionRead()
{ }

// from test-api.cc:3223
void
test_IndexedInterceptorWithIndexedAccessor()
{ }

// from test-api.cc:3260
void
test_IndexedInterceptorWithGetOwnPropertyDescriptor()
{ }

// from test-api.cc:3281
void
test_IndexedInterceptorWithNoSetter()
{ }

// from test-api.cc:3304
void
test_IndexedInterceptorWithAccessorCheck()
{ }

// from test-api.cc:3328
void
test_IndexedInterceptorWithAccessorCheckSwitchedOn()
{ }

// from test-api.cc:3358
void
test_IndexedInterceptorWithDifferentIndices()
{ }

// from test-api.cc:3381
void
test_IndexedInterceptorWithNegativeIndices()
{ }

// from test-api.cc:3420
void
test_IndexedInterceptorWithNotSmiLookup()
{ }

// from test-api.cc:3449
void
test_IndexedInterceptorGoingMegamorphic()
{ }

// from test-api.cc:3479
void
test_IndexedInterceptorReceiverTurningSmi()
{ }

// from test-api.cc:3509
void
test_IndexedInterceptorOnProto()
{ }

// from test-api.cc:3533
void
test_MultiContexts()
{ }

// from test-api.cc:3566
void
test_FunctionPrototypeAcrossContexts()
{ }

// from test-api.cc:3596
void
test_Regress892105()
{ }

// from test-api.cc:3619
void
test_UndetectableObject()
{ }

// from test-api.cc:3664
void
test_ExtensibleOnUndetectable()
{ }

// from test-api.cc:3699
void
test_UndetectableString()
{ }

// from test-api.cc:3764
void
test_GlobalObjectTemplate()
{ }

// from test-api.cc:3782
void
test_SimpleExtensions()
{ }

// from test-api.cc:3811
void
test_UseEvalFromExtension()
{ }

// from test-api.cc:3844
void
test_UseWithFromExtension()
{ }

// from test-api.cc:3859
void
test_AutoExtensions()
{ }

// from test-api.cc:3877
void
test_SyntaxErrorExtensions()
{ }

// from test-api.cc:3894
void
test_ExceptionExtensions()
{ }

// from test-api.cc:3915
void
test_NativeCallInExtensions()
{ }

// from test-api.cc:3943
void
test_ExtensionDependency()
{ }

// from test-api.cc:4010
void
test_FunctionLookup()
{ }

// from test-api.cc:4023
void
test_NativeFunctionConstructCall()
{ }

// from test-api.cc:4055
void
test_ErrorReporting()
{ }

// from test-api.cc:4082
void
test_RegexpOutOfMemory()
{ }

// from test-api.cc:4106
void
test_ErrorWithMissingScriptInfo()
{ }

// from test-api.cc:4169
void
test_WeakReference()
{ }

// from test-api.cc:4222
void
test_NoWeakRefCallbacksInScavenge()
{ }

// from test-api.cc:4268
void
test_Arguments()
{ }

// from test-api.cc:4309
void
test_Deleter()
{ }

// from test-api.cc:4372
void
test_Enumerators()
{ }

// from test-api.cc:4486
void
test_GetterHolders()
{ }

// from test-api.cc:4499
void
test_PreInterceptorHolders()
{ }

// from test-api.cc:4509
void
test_ObjectInstantiation()
{ }

// from test-api.cc:4549
void
test_StringWrite()
{
  v8::HandleScope scope;
  v8::Handle<String> str = v8_str("abcde");
  // abc<Icelandic eth><Unicode snowman>.
  v8::Handle<String> str2 = v8_str("abc\303\260\342\230\203");

  CHECK_EQ(5, str2->Length());

  char buf[100];
  char utf8buf[100];
  uint16_t wbuf[100];
  int len;
  int charlen;

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, sizeof(utf8buf), &charlen);
  CHECK_EQ(9, len);
  CHECK_EQ(5, charlen);
  CHECK_EQ(0, strcmp(utf8buf, "abc\303\260\342\230\203"));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 8, &charlen);
  CHECK_EQ(8, len);
  CHECK_EQ(5, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\342\230\203\1", 9));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 7, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\1", 5));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 6, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\1", 5));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 5, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\303\260\1", 5));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 4, &charlen);
  CHECK_EQ(3, len);
  CHECK_EQ(3, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\1", 4));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 3, &charlen);
  CHECK_EQ(3, len);
  CHECK_EQ(3, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\1", 4));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 2, &charlen);
  CHECK_EQ(2, len);
  CHECK_EQ(2, charlen);
  CHECK_EQ(2, strncmp(utf8buf, "ab\1", 3));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf);
  CHECK_EQ(5, len);
  len = str->Write(wbuf);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));
  uint16_t answer1[] = {'a', 'b', 'c', 'd', 'e', '\0'};
  CHECK_EQ(0, StrCmp16(answer1, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 0, 4);
  CHECK_EQ(4, len);
  len = str->Write(wbuf, 0, 4);
  CHECK_EQ(4, len);
  CHECK_EQ(0, strncmp("abcd\1", buf, 5));
  uint16_t answer2[] = {'a', 'b', 'c', 'd', 0x101};
  CHECK_EQ(0, StrNCmp16(answer2, wbuf, 5));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 0, 5);
  CHECK_EQ(5, len);
  len = str->Write(wbuf, 0, 5);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strncmp("abcde\1", buf, 6));
  uint16_t answer3[] = {'a', 'b', 'c', 'd', 'e', 0x101};
  CHECK_EQ(0, StrNCmp16(answer3, wbuf, 6));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 0, 6);
  CHECK_EQ(5, len);
  len = str->Write(wbuf, 0, 6);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));
  uint16_t answer4[] = {'a', 'b', 'c', 'd', 'e', '\0'};
  CHECK_EQ(0, StrCmp16(answer4, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 4, -1);
  CHECK_EQ(1, len);
  len = str->Write(wbuf, 4, -1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));
  uint16_t answer5[] = {'e', '\0'};
  CHECK_EQ(0, StrCmp16(answer5, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 4, 6);
  CHECK_EQ(1, len);
  len = str->Write(wbuf, 4, 6);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));
  CHECK_EQ(0, StrCmp16(answer5, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 4, 1);
  CHECK_EQ(1, len);
  len = str->Write(wbuf, 4, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("e\1", buf, 2));
  uint16_t answer6[] = {'e', 0x101};
  CHECK_EQ(0, StrNCmp16(answer6, wbuf, 2));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 3, 1);
  CHECK_EQ(1, len);
  len = str->Write(wbuf, 3, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(1, strncmp("d\1", buf, 2));
  uint16_t answer7[] = {'d', 0x101};
  CHECK_EQ(0, StrNCmp16(answer7, wbuf, 2));
}

// from test-api.cc:4692
void
test_ToArrayIndex()
{ }

// from test-api.cc:4723
void
test_ErrorConstruction()
{ }

// from test-api.cc:4764
void
test_DeleteAccessor()
{ }

// from test-api.cc:4777
void
test_TypeSwitch()
{ }

// from test-api.cc:4854
void
test_ApiUncaughtException()
{ }

// from test-api.cc:4894
void
test_ExceptionInNativeScript()
{ }

// from test-api.cc:4914
void
test_CompilationErrorUsingTryCatchHandler()
{ }

// from test-api.cc:4924
void
test_TryCatchFinallyUsingTryCatchHandler()
{ }

// from test-api.cc:4945
void
test_SecurityHandler()
{ }

// from test-api.cc:5007
void
test_SecurityChecks()
{ }

// from test-api.cc:5052
void
test_SecurityChecksForPrototypeChain()
{ }

// from test-api.cc:5120
void
test_CrossDomainDelete()
{ }

// from test-api.cc:5153
void
test_CrossDomainIsPropertyEnumerable()
{ }

// from test-api.cc:5188
void
test_CrossDomainForIn()
{ }

// from test-api.cc:5221
void
test_ContextDetachGlobal()
{ }

// from test-api.cc:5285
void
test_DetachAndReattachGlobal()
{ }

// from test-api.cc:5411
void
test_AccessControl()
{ }

// from test-api.cc:5655
void
test_AccessControlES5()
{ }

// from test-api.cc:5721
void
test_AccessControlGetOwnPropertyNames()
{ }

// from test-api.cc:5771
void
test_GetOwnPropertyNamesWithInterceptor()
{ }

// from test-api.cc:5795
void
test_CrossDomainAccessors()
{ }

// from test-api.cc:5870
void
test_AccessControlIC()
{ }

// from test-api.cc:6019
void
test_AccessControlFlatten()
{ }

// from test-api.cc:6083
void
test_AccessControlInterceptorIC()
{ }

// from test-api.cc:6150
void
test_Version()
{ }

// from test-api.cc:6161
void
test_InstanceProperties()
{ }

// from test-api.cc:6190
void
test_GlobalObjectInstanceProperties()
{ }

// from test-api.cc:6246
void
test_CallKnownGlobalReceiver()
{ }

// from test-api.cc:6323
void
test_ShadowObject()
{ }

// from test-api.cc:6368
void
test_HiddenPrototype()
{ }

// from test-api.cc:6412
void
test_SetPrototype()
{ }

// from test-api.cc:6469
void
test_SetPrototypeThrows()
{ }

// from test-api.cc:6490
void
test_GetterSetterExceptions()
{ }

// from test-api.cc:6513
void
test_Constructor()
{ }

// from test-api.cc:6526
void
test_FunctionDescriptorException()
{ }

// from test-api.cc:6551
void
test_EvalAliasedDynamic()
{ }

// from test-api.cc:6585
void
test_CrossEval()
{ }

// from test-api.cc:6668
void
test_EvalInDetachedGlobal()
{ }

// from test-api.cc:6703
void
test_CrossLazyLoad()
{ }

// from test-api.cc:6738
void
test_CallAsFunction()
{ }

// from test-api.cc:6804
void
test_HandleIteration()
{ }

// from test-api.cc:6839
void
test_InterceptorHasOwnProperty()
{ }

// from test-api.cc:6871
void
test_InterceptorHasOwnPropertyCausingGC()
{ }

// from test-api.cc:6927
void
test_InterceptorLoadIC()
{ }

// from test-api.cc:6949
void
test_InterceptorLoadICWithFieldOnHolder()
{ }

// from test-api.cc:6960
void
test_InterceptorLoadICWithSubstitutedProto()
{ }

// from test-api.cc:6971
void
test_InterceptorLoadICWithPropertyOnProto()
{ }

// from test-api.cc:6982
void
test_InterceptorLoadICUndefined()
{ }

// from test-api.cc:6992
void
test_InterceptorLoadICWithOverride()
{ }

// from test-api.cc:7012
void
test_InterceptorLoadICFieldNotNeeded()
{ }

// from test-api.cc:7032
void
test_InterceptorLoadICInvalidatedField()
{ }

// from test-api.cc:7063
void
test_InterceptorLoadICPostInterceptor()
{ }

// from test-api.cc:7088
void
test_InterceptorLoadICInvalidatedFieldViaGlobal()
{ }

// from test-api.cc:7113
void
test_InterceptorLoadICWithCallbackOnHolder()
{ }

// from test-api.cc:7142
void
test_InterceptorLoadICWithCallbackOnProto()
{ }

// from test-api.cc:7175
void
test_InterceptorLoadICForCallbackWithOverride()
{ }

// from test-api.cc:7203
void
test_InterceptorLoadICCallbackNotNeeded()
{ }

// from test-api.cc:7231
void
test_InterceptorLoadICInvalidatedCallback()
{ }

// from test-api.cc:7263
void
test_InterceptorLoadICInvalidatedCallbackViaGlobal()
{ }

// from test-api.cc:7299
void
test_InterceptorReturningZero()
{ }

// from test-api.cc:7315
void
test_InterceptorStoreIC()
{ }

// from test-api.cc:7330
void
test_InterceptorStoreICWithNoSetter()
{ }

// from test-api.cc:7360
void
test_InterceptorCallIC()
{ }

// from test-api.cc:7379
void
test_InterceptorCallICSeesOthers()
{ }

// from test-api.cc:7407
void
test_InterceptorCallICCacheableNotNeeded()
{ }

// from test-api.cc:7427
void
test_InterceptorCallICInvalidatedCacheable()
{ }

// from test-api.cc:7465
void
test_InterceptorCallICConstantFunctionUsed()
{ }

// from test-api.cc:7486
void
test_InterceptorCallICConstantFunctionNotNeeded()
{ }

// from test-api.cc:7508
void
test_InterceptorCallICInvalidatedConstantFunction()
{ }

// from test-api.cc:7538
void
test_InterceptorCallICInvalidatedConstantFunctionViaGlobal()
{ }

// from test-api.cc:7563
void
test_InterceptorCallICCachedFromGlobal()
{ }

// from test-api.cc:7641
void
test_CallICFastApi_DirectCall_GCMoveStub()
{ }

// from test-api.cc:7665
void
test_CallICFastApi_DirectCall_Throw()
{ }

// from test-api.cc:7696
void
test_LoadICFastApi_DirectCall_GCMoveStub()
{ }

// from test-api.cc:7718
void
test_LoadICFastApi_DirectCall_Throw()
{ }

// from test-api.cc:7734
void
test_InterceptorCallICFastApi_TrivialSignature()
{ }

// from test-api.cc:7761
void
test_InterceptorCallICFastApi_SimpleSignature()
{ }

// from test-api.cc:7791
void
test_InterceptorCallICFastApi_SimpleSignature_Miss1()
{ }

// from test-api.cc:7827
void
test_InterceptorCallICFastApi_SimpleSignature_Miss2()
{ }

// from test-api.cc:7863
void
test_InterceptorCallICFastApi_SimpleSignature_Miss3()
{ }

// from test-api.cc:7902
void
test_InterceptorCallICFastApi_SimpleSignature_TypeError()
{ }

// from test-api.cc:7941
void
test_CallICFastApi_TrivialSignature()
{ }

// from test-api.cc:7964
void
test_CallICFastApi_SimpleSignature()
{ }

// from test-api.cc:7990
void
test_CallICFastApi_SimpleSignature_Miss1()
{ }

// from test-api.cc:8021
void
test_CallICFastApi_SimpleSignature_Miss2()
{ }

// from test-api.cc:8070
void
test_InterceptorKeyedCallICKeyChange1()
{ }

// from test-api.cc:8094
void
test_InterceptorKeyedCallICKeyChange2()
{ }

// from test-api.cc:8121
void
test_InterceptorKeyedCallICKeyChangeOnGlobal()
{ }

// from test-api.cc:8146
void
test_InterceptorKeyedCallICFromGlobal()
{ }

// from test-api.cc:8170
void
test_InterceptorKeyedCallICMapChangeBefore()
{ }

// from test-api.cc:8192
void
test_InterceptorKeyedCallICMapChangeAfter()
{ }

// from test-api.cc:8228
void
test_InterceptorICReferenceErrors()
{ }

// from test-api.cc:8274
void
test_InterceptorICGetterExceptions()
{ }

// from test-api.cc:8317
void
test_InterceptorICSetterExceptions()
{ }

// from test-api.cc:8336
void
test_NullNamedInterceptor()
{ }

// from test-api.cc:8351
void
test_NullIndexedInterceptor()
{ }

// from test-api.cc:8365
void
test_NamedPropertyHandlerGetterAttributes()
{ }

// from test-api.cc:8391
void
test_Overriding()
{ }

// from test-api.cc:8457
void
test_IsConstructCall()
{ }

// from test-api.cc:8474
void
test_ObjectProtoToString()
{ }

// from test-api.cc:8508
void
test_ObjectGetConstructorName()
{ }

// from test-api.cc:8659
void
test_Threading()
{ }

// from test-api.cc:8665
void
test_Threading2()
{ }

// from test-api.cc:8721
void
test_NestedLockers()
{ }

// from test-api.cc:8743
void
test_NestedLockersNoTryCatch()
{ }

// from test-api.cc:8763
void
test_RecursiveLocking()
{ }

// from test-api.cc:8779
void
test_LockUnlockLock()
{ }

// from test-api.cc:8836
void
test_DontLeakGlobalObjects()
{ }

// from test-api.cc:8880
void
test_NewPersistentHandleFromWeakCallback()
{ }

// from test-api.cc:8909
void
test_DoNotUseDeletedNodesInSecondLevelGc()
{ }

// from test-api.cc:8934
void
test_NoGlobalHandlesOrphaningDueToWeakCallback()
{ }

// from test-api.cc:8950
void
test_CheckForCrossContextObjectLiterals()
{ }

// from test-api.cc:8983
void
test_NestedHandleScopeAndContexts()
{ }

// from test-api.cc:8994
void
test_ExternalAllocatedMemory()
{ }

// from test-api.cc:9003
void
test_DisposeEnteredContext()
{ }

// from test-api.cc:9018
void
test_Regress54()
{ }

// from test-api.cc:9035
void
test_CatchStackOverflow()
{ }

// from test-api.cc:9072
void
test_TryCatchSourceInfo()
{ }

// from test-api.cc:9108
void
test_CompilationCache()
{ }

// from test-api.cc:9131
void
test_CallbackFunctionName()
{ }

// from test-api.cc:9144
void
test_DateAccess()
{
  v8::HandleScope scope;
  LocalContext context;
  v8::Handle<v8::Value> date = v8::Date::New(1224744689038.0);
  CHECK(date->IsDate());
  CHECK_EQ(1224744689038.0, date.As<v8::Date>()->NumberValue());
}

// from test-api.cc:9164
void
test_PropertyEnumeration()
{
  v8::HandleScope scope;
  LocalContext context;
  v8::Handle<v8::Value> obj = v8::Script::Compile(v8::String::New(
      "var result = [];"
      "result[0] = {};"
      "result[1] = {a: 1, b: 2};"
      "result[2] = [1, 2, 3];"
      "var proto = {x: 1, y: 2, z: 3};"
      "var x = { __proto__: proto, w: 0, z: 1 };"
      "result[3] = x;"
      "result;"))->Run();
  v8::Handle<v8::Array> elms = obj.As<v8::Array>();
  CHECK_EQ(4, elms->Length());
  int elmc0 = 0;
  const char** elmv0 = NULL;
  CheckProperties(elms->Get(v8::Integer::New(0)), elmc0, elmv0);
  int elmc1 = 2;
  const char* elmv1[] = {"a", "b"};
  CheckProperties(elms->Get(v8::Integer::New(1)), elmc1, elmv1);
  int elmc2 = 3;
  const char* elmv2[] = {"0", "1", "2"};
  CheckProperties(elms->Get(v8::Integer::New(2)), elmc2, elmv2);
  int elmc3 = 4;
  const char* elmv3[] = {"w", "z", "x", "y"};
  CheckProperties(elms->Get(v8::Integer::New(3)), elmc3, elmv3);
}

// from test-api.cc:9209
void
test_DisableAccessChecksWhileConfiguring()
{ }

// from test-api.cc:9240
void
test_AccessChecksReenabledCorrectly()
{ }

// from test-api.cc:9279
void
test_AccessControlRepeatedContextCreation()
{ }

// from test-api.cc:9295
void
test_TurnOnAccessCheck()
{ }

// from test-api.cc:9371
void
test_TurnOnAccessCheckAndRecompile()
{ }

// from test-api.cc:9462
void
test_PreCompile()
{ }

// from test-api.cc:9476
void
test_PreCompileWithError()
{ }

// from test-api.cc:9486
void
test_Regress31661()
{ }

// from test-api.cc:9497
void
test_PreCompileSerialization()
{ }

// from test-api.cc:9523
void
test_PreCompileDeserializationError()
{ }

// from test-api.cc:9536
void
test_PreCompileInvalidPreparseDataError()
{ }

// from test-api.cc:9584
void
test_PreCompileAPIVariationsAreSame()
{ }

// from test-api.cc:9622
void
test_DictionaryICLoadedFunction()
{ }

// from test-api.cc:9643
void
test_CrossContextNew()
{ }

// from test-api.cc:9775
void
test_RegExpInterruption()
{ }

// from test-api.cc:9883
void
test_ApplyInterruption()
{ }

// from test-api.cc:9905
void
test_ObjectClone()
{ }

// from test-api.cc:9988
void
test_MorphCompositeStringTest()
{ }

// from test-api.cc:10041
void
test_CompileExternalTwoByteSource()
{ }

// from test-api.cc:10181
void
test_RegExpStringModification()
{ }

// from test-api.cc:10204
void
test_ReadOnlyPropertyInGlobalProto()
{ }

// from test-api.cc:10252
void
test_ForceSet()
{ }

// from test-api.cc:10293
void
test_ForceSetWithInterceptor()
{ }

// from test-api.cc:10340
void
test_ForceDelete()
{ }

// from test-api.cc:10375
void
test_ForceDeleteWithInterceptor()
{ }

// from test-api.cc:10410
void
test_ForceDeleteIC()
{ }

// from test-api.cc:10447
void
test_GetCallingContext()
{ }

// from test-api.cc:10500
void
test_InitGlobalVarInProtoChain()
{ }

// from test-api.cc:10516
void
test_ReplaceConstantFunction()
{ }

// from test-api.cc:10530
void
test_Regress16276()
{ }

// from test-api.cc:10547
void
test_PixelArray()
{ }

// from test-api.cc:10921
void
test_PixelArrayInfo()
{ }

// from test-api.cc:10953
void
test_PixelArrayWithInterceptor()
{ }

// from test-api.cc:11361
void
test_ExternalByteArray()
{ }

// from test-api.cc:11369
void
test_ExternalUnsignedByteArray()
{ }

// from test-api.cc:11377
void
test_ExternalShortArray()
{ }

// from test-api.cc:11385
void
test_ExternalUnsignedShortArray()
{ }

// from test-api.cc:11393
void
test_ExternalIntArray()
{ }

// from test-api.cc:11401
void
test_ExternalUnsignedIntArray()
{ }

// from test-api.cc:11409
void
test_ExternalFloatArray()
{ }

// from test-api.cc:11417
void
test_ExternalArrays()
{ }

// from test-api.cc:11446
void
test_ExternalArrayInfo()
{ }

// from test-api.cc:11457
void
test_ScriptContextDependence()
{ }

// from test-api.cc:11473
void
test_StackTrace()
{ }

// from test-api.cc:11562
void
test_CaptureStackTrace()
{ }

// from test-api.cc:11620
void
test_CaptureStackTraceForUncaughtException()
{ }

// from test-api.cc:11643
void
test_CaptureStackTraceForUncaughtExceptionAndSetters()
{ }

// from test-api.cc:11679
void
test_SourceURLInStackTrace()
{ }

// from test-api.cc:11703
void
test_IdleNotification()
{
  bool rv = false;
  for (int i = 0; i < 100; i++) {
    rv = v8::V8::IdleNotification();
    if (rv)
      break;
  }
  CHECK(rv == true);
}

// from test-api.cc:11736
void
test_SetResourceConstraints()
{ }

// from test-api.cc:11758
void
test_SetResourceConstraintsInThread()
{ }

// from test-api.cc:11788
void
test_GetHeapStatistics()
{ }

// from test-api.cc:11843
void
test_QuietSignalingNaNs()
{ }

// from test-api.cc:11927
void
test_SpaghettiStackReThrow()
{ }

// from test-api.cc:11953
void
test_Regress528()
{ }

// from test-api.cc:12042
void
test_ScriptOrigin()
{ }

// from test-api.cc:12064
void
test_ScriptLineNumber()
{ }

// from test-api.cc:12093
void
test_SetterOnConstructorPrototype()
{ }

// from test-api.cc:12143
void
test_InterceptorOnConstructorPrototype()
{ }

// from test-api.cc:12177
void
test_Bug618()
{ }

// from test-api.cc:12243
void
test_GCCallbacks()
{ }

// from test-api.cc:12277
void
test_AddToJSFunctionResultCache()
{ }

// from test-api.cc:12304
void
test_FillJSFunctionResultCache()
{ }

// from test-api.cc:12326
void
test_RoundRobinGetFromCache()
{ }

// from test-api.cc:12351
void
test_ReverseGetFromCache()
{ }

// from test-api.cc:12376
void
test_TestEviction()
{ }

// from test-api.cc:12394
void
test_TwoByteStringInAsciiCons()
{ }

// from test-api.cc:12483
void
test_GCInFailedAccessCheckCallback()
{ }

// from test-api.cc:12561
void
test_StringCheckMultipleContexts()
{ }

// from test-api.cc:12584
void
test_NumberCheckMultipleContexts()
{ }

// from test-api.cc:12607
void
test_BooleanCheckMultipleContexts()
{ }

// from test-api.cc:12630
void
test_DontDeleteCellLoadIC()
{ }

// from test-api.cc:12670
void
test_DontDeleteCellLoadICForceDelete()
{ }

// from test-api.cc:12698
void
test_DontDeleteCellLoadICAPI()
{ }

// from test-api.cc:12726
void
test_GlobalLoadICGC()
{ }

// from test-api.cc:12768
void
test_RegExp()
{ }

// from test-api.cc:12833
void
test_Equals()
{
  v8::HandleScope handleScope;
  LocalContext localContext;

  v8::Handle<v8::Object> globalProxy = localContext->Global();
  v8::Handle<Value> global = globalProxy->GetPrototype();

  CHECK(global->StrictEquals(global));
  CHECK(!global->StrictEquals(globalProxy));
  CHECK(!globalProxy->StrictEquals(global));
  CHECK(globalProxy->StrictEquals(globalProxy));

  CHECK(global->Equals(global));
  CHECK(!global->Equals(globalProxy));
  CHECK(!globalProxy->Equals(global));
  CHECK(globalProxy->Equals(globalProxy));
}

// from test-api.cc:12865
void
test_NamedEnumeratorAndForIn()
{ }

// from test-api.cc:12880
void
test_DefinePropertyPostDetach()
{ }


////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_Handles),
  UNIMPLEMENTED_TEST(test_ReceiverSignature),
  UNIMPLEMENTED_TEST(test_ArgumentSignature),
  DISABLED_TEST(test_HulIgennem, 12),
  TEST(test_Access),
  TEST(test_AccessElement),
  TEST(test_Script),
  UNIMPLEMENTED_TEST(test_ScriptUsingStringResource),
  UNIMPLEMENTED_TEST(test_ScriptUsingAsciiStringResource),
  UNIMPLEMENTED_TEST(test_ScriptMakingExternalString),
  UNIMPLEMENTED_TEST(test_ScriptMakingExternalAsciiString),
  UNIMPLEMENTED_TEST(test_MakingExternalStringConditions),
  UNIMPLEMENTED_TEST(test_MakingExternalAsciiStringConditions),
  UNIMPLEMENTED_TEST(test_UsingExternalString),
  UNIMPLEMENTED_TEST(test_UsingExternalAsciiString),
  UNIMPLEMENTED_TEST(test_ScavengeExternalString),
  UNIMPLEMENTED_TEST(test_ScavengeExternalAsciiString),
  UNIMPLEMENTED_TEST(test_ExternalStringWithDisposeHandling),
  UNIMPLEMENTED_TEST(test_StringConcat),
  TEST(test_GlobalProperties),
  UNIMPLEMENTED_TEST(test_FunctionTemplate),
  UNIMPLEMENTED_TEST(test_ExternalWrap),
  UNIMPLEMENTED_TEST(test_FindInstanceInPrototypeChain),
  TEST(test_TinyInteger),
  UNIMPLEMENTED_TEST(test_BigSmiInteger),
  UNIMPLEMENTED_TEST(test_BigInteger),
  TEST(test_TinyUnsignedInteger),
  UNIMPLEMENTED_TEST(test_BigUnsignedSmiInteger),
  UNIMPLEMENTED_TEST(test_BigUnsignedInteger),
  TEST(test_OutOfSignedRangeUnsignedInteger),
  TEST(test_Number),
  TEST(test_ToNumber),
  TEST(test_Date),
  TEST(test_Boolean),
  UNIMPLEMENTED_TEST(test_GlobalPrototype),
  UNIMPLEMENTED_TEST(test_ObjectTemplate),
  UNIMPLEMENTED_TEST(test_DescriptorInheritance),
  UNIMPLEMENTED_TEST(test_NamedPropertyHandlerGetter),
  UNIMPLEMENTED_TEST(test_IndexedPropertyHandlerGetter),
  UNIMPLEMENTED_TEST(test_PropertyHandlerInPrototype),
  UNIMPLEMENTED_TEST(test_PrePropertyHandler),
  UNIMPLEMENTED_TEST(test_UndefinedIsNotEnumerable),
  UNIMPLEMENTED_TEST(test_DeepCrossLanguageRecursion),
  UNIMPLEMENTED_TEST(test_CallbackExceptionRegression),
  UNIMPLEMENTED_TEST(test_FunctionPrototype),
  UNIMPLEMENTED_TEST(test_InternalFields),
  UNIMPLEMENTED_TEST(test_GlobalObjectInternalFields),
  UNIMPLEMENTED_TEST(test_InternalFieldsNativePointers),
  UNIMPLEMENTED_TEST(test_InternalFieldsNativePointersAndExternal),
  UNIMPLEMENTED_TEST(test_IdentityHash),
  UNIMPLEMENTED_TEST(test_HiddenProperties),
  UNIMPLEMENTED_TEST(test_HiddenPropertiesWithInterceptors),
  UNIMPLEMENTED_TEST(test_External),
  TEST(test_GlobalHandle),
  TEST(test_ScriptException),
  UNIMPLEMENTED_TEST(test_MessageHandlerData),
  TEST(test_GetSetProperty),
  TEST(test_PropertyAttributes),
  TEST(test_Array),
  UNIMPLEMENTED_TEST(test_Vector),
  TEST(test_FunctionCall),
  UNIMPLEMENTED_TEST(test_OutOfMemory),
  UNIMPLEMENTED_TEST(test_OutOfMemoryNested),
  UNIMPLEMENTED_TEST(test_HugeConsStringOutOfMemory),
  UNIMPLEMENTED_TEST(test_ConstructCall),
  TEST(test_ConversionNumber),
  TEST(test_isNumberType),
  UNIMPLEMENTED_TEST(test_ConversionException),
  UNIMPLEMENTED_TEST(test_APICatch),
  UNIMPLEMENTED_TEST(test_APIThrowTryCatch),
  UNIMPLEMENTED_TEST(test_TryCatchInTryFinally),
  UNIMPLEMENTED_TEST(test_APIThrowMessageOverwrittenToString),
  UNIMPLEMENTED_TEST(test_APIThrowMessage),
  UNIMPLEMENTED_TEST(test_APIThrowMessageAndVerboseTryCatch),
  UNIMPLEMENTED_TEST(test_ExternalScriptException),
  TEST(test_EvalInTryFinally),
  UNIMPLEMENTED_TEST(test_ExceptionOrder),
  UNIMPLEMENTED_TEST(test_ThrowValues),
  TEST(test_CatchZero),
  TEST(test_CatchExceptionFromWith),
  TEST(test_TryCatchAndFinallyHidingException),
  UNIMPLEMENTED_TEST(test_TryCatchAndFinally),
  UNIMPLEMENTED_TEST(test_Equality),
  UNIMPLEMENTED_TEST(test_MultiRun),
  DISABLED_TEST(test_SimplePropertyRead, 26),
  UNIMPLEMENTED_TEST(test_DefinePropertyOnAPIAccessor),
  UNIMPLEMENTED_TEST(test_DefinePropertyOnDefineGetterSetter),
  UNIMPLEMENTED_TEST(test_DefineAPIAccessorOnObject),
  UNIMPLEMENTED_TEST(test_DontDeleteAPIAccessorsCannotBeOverriden),
  UNIMPLEMENTED_TEST(test_ElementAPIAccessor),
  UNIMPLEMENTED_TEST(test_SimplePropertyWrite),
  UNIMPLEMENTED_TEST(test_NamedInterceptorPropertyRead),
  UNIMPLEMENTED_TEST(test_NamedInterceptorDictionaryIC),
  UNIMPLEMENTED_TEST(test_NamedInterceptorDictionaryICMultipleContext),
  UNIMPLEMENTED_TEST(test_NamedInterceptorMapTransitionRead),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithIndexedAccessor),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithGetOwnPropertyDescriptor),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithNoSetter),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithAccessorCheck),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithAccessorCheckSwitchedOn),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithDifferentIndices),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithNegativeIndices),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorWithNotSmiLookup),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorGoingMegamorphic),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorReceiverTurningSmi),
  UNIMPLEMENTED_TEST(test_IndexedInterceptorOnProto),
  UNIMPLEMENTED_TEST(test_MultiContexts),
  UNIMPLEMENTED_TEST(test_FunctionPrototypeAcrossContexts),
  UNIMPLEMENTED_TEST(test_Regress892105),
  UNIMPLEMENTED_TEST(test_UndetectableObject),
  UNIMPLEMENTED_TEST(test_ExtensibleOnUndetectable),
  UNIMPLEMENTED_TEST(test_UndetectableString),
  UNIMPLEMENTED_TEST(test_GlobalObjectTemplate),
  UNIMPLEMENTED_TEST(test_SimpleExtensions),
  UNIMPLEMENTED_TEST(test_UseEvalFromExtension),
  UNIMPLEMENTED_TEST(test_UseWithFromExtension),
  UNIMPLEMENTED_TEST(test_AutoExtensions),
  UNIMPLEMENTED_TEST(test_SyntaxErrorExtensions),
  UNIMPLEMENTED_TEST(test_ExceptionExtensions),
  UNIMPLEMENTED_TEST(test_NativeCallInExtensions),
  UNIMPLEMENTED_TEST(test_ExtensionDependency),
  UNIMPLEMENTED_TEST(test_FunctionLookup),
  UNIMPLEMENTED_TEST(test_NativeFunctionConstructCall),
  UNIMPLEMENTED_TEST(test_ErrorReporting),
  UNIMPLEMENTED_TEST(test_RegexpOutOfMemory),
  UNIMPLEMENTED_TEST(test_ErrorWithMissingScriptInfo),
  UNIMPLEMENTED_TEST(test_WeakReference),
  UNIMPLEMENTED_TEST(test_NoWeakRefCallbacksInScavenge),
  UNIMPLEMENTED_TEST(test_Arguments),
  UNIMPLEMENTED_TEST(test_Deleter),
  UNIMPLEMENTED_TEST(test_Enumerators),
  UNIMPLEMENTED_TEST(test_GetterHolders),
  UNIMPLEMENTED_TEST(test_PreInterceptorHolders),
  UNIMPLEMENTED_TEST(test_ObjectInstantiation),
  DISABLED_TEST(test_StringWrite, 16),
  UNIMPLEMENTED_TEST(test_ToArrayIndex),
  UNIMPLEMENTED_TEST(test_ErrorConstruction),
  UNIMPLEMENTED_TEST(test_DeleteAccessor),
  UNIMPLEMENTED_TEST(test_TypeSwitch),
  UNIMPLEMENTED_TEST(test_ApiUncaughtException),
  UNIMPLEMENTED_TEST(test_ExceptionInNativeScript),
  UNIMPLEMENTED_TEST(test_CompilationErrorUsingTryCatchHandler),
  UNIMPLEMENTED_TEST(test_TryCatchFinallyUsingTryCatchHandler),
  UNIMPLEMENTED_TEST(test_SecurityHandler),
  UNIMPLEMENTED_TEST(test_SecurityChecks),
  UNIMPLEMENTED_TEST(test_SecurityChecksForPrototypeChain),
  UNIMPLEMENTED_TEST(test_CrossDomainDelete),
  UNIMPLEMENTED_TEST(test_CrossDomainIsPropertyEnumerable),
  UNIMPLEMENTED_TEST(test_CrossDomainForIn),
  UNIMPLEMENTED_TEST(test_ContextDetachGlobal),
  UNIMPLEMENTED_TEST(test_DetachAndReattachGlobal),
  UNIMPLEMENTED_TEST(test_AccessControl),
  UNIMPLEMENTED_TEST(test_AccessControlES5),
  UNIMPLEMENTED_TEST(test_AccessControlGetOwnPropertyNames),
  UNIMPLEMENTED_TEST(test_GetOwnPropertyNamesWithInterceptor),
  UNIMPLEMENTED_TEST(test_CrossDomainAccessors),
  UNIMPLEMENTED_TEST(test_AccessControlIC),
  UNIMPLEMENTED_TEST(test_AccessControlFlatten),
  UNIMPLEMENTED_TEST(test_AccessControlInterceptorIC),
  UNIMPLEMENTED_TEST(test_Version),
  UNIMPLEMENTED_TEST(test_InstanceProperties),
  UNIMPLEMENTED_TEST(test_GlobalObjectInstanceProperties),
  UNIMPLEMENTED_TEST(test_CallKnownGlobalReceiver),
  UNIMPLEMENTED_TEST(test_ShadowObject),
  UNIMPLEMENTED_TEST(test_HiddenPrototype),
  UNIMPLEMENTED_TEST(test_SetPrototype),
  UNIMPLEMENTED_TEST(test_SetPrototypeThrows),
  UNIMPLEMENTED_TEST(test_GetterSetterExceptions),
  UNIMPLEMENTED_TEST(test_Constructor),
  UNIMPLEMENTED_TEST(test_FunctionDescriptorException),
  UNIMPLEMENTED_TEST(test_EvalAliasedDynamic),
  UNIMPLEMENTED_TEST(test_CrossEval),
  UNIMPLEMENTED_TEST(test_EvalInDetachedGlobal),
  UNIMPLEMENTED_TEST(test_CrossLazyLoad),
  UNIMPLEMENTED_TEST(test_CallAsFunction),
  UNIMPLEMENTED_TEST(test_HandleIteration),
  UNIMPLEMENTED_TEST(test_InterceptorHasOwnProperty),
  UNIMPLEMENTED_TEST(test_InterceptorHasOwnPropertyCausingGC),
  UNIMPLEMENTED_TEST(test_InterceptorLoadIC),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICWithFieldOnHolder),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICWithSubstitutedProto),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICWithPropertyOnProto),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICUndefined),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICWithOverride),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICFieldNotNeeded),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICInvalidatedField),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICPostInterceptor),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICInvalidatedFieldViaGlobal),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICWithCallbackOnHolder),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICWithCallbackOnProto),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICForCallbackWithOverride),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICCallbackNotNeeded),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICInvalidatedCallback),
  UNIMPLEMENTED_TEST(test_InterceptorLoadICInvalidatedCallbackViaGlobal),
  UNIMPLEMENTED_TEST(test_InterceptorReturningZero),
  UNIMPLEMENTED_TEST(test_InterceptorStoreIC),
  UNIMPLEMENTED_TEST(test_InterceptorStoreICWithNoSetter),
  UNIMPLEMENTED_TEST(test_InterceptorCallIC),
  UNIMPLEMENTED_TEST(test_InterceptorCallICSeesOthers),
  UNIMPLEMENTED_TEST(test_InterceptorCallICCacheableNotNeeded),
  UNIMPLEMENTED_TEST(test_InterceptorCallICInvalidatedCacheable),
  UNIMPLEMENTED_TEST(test_InterceptorCallICConstantFunctionUsed),
  UNIMPLEMENTED_TEST(test_InterceptorCallICConstantFunctionNotNeeded),
  UNIMPLEMENTED_TEST(test_InterceptorCallICInvalidatedConstantFunction),
  UNIMPLEMENTED_TEST(test_InterceptorCallICInvalidatedConstantFunctionViaGlobal),
  UNIMPLEMENTED_TEST(test_InterceptorCallICCachedFromGlobal),
  UNIMPLEMENTED_TEST(test_CallICFastApi_DirectCall_GCMoveStub),
  UNIMPLEMENTED_TEST(test_CallICFastApi_DirectCall_Throw),
  UNIMPLEMENTED_TEST(test_LoadICFastApi_DirectCall_GCMoveStub),
  UNIMPLEMENTED_TEST(test_LoadICFastApi_DirectCall_Throw),
  UNIMPLEMENTED_TEST(test_InterceptorCallICFastApi_TrivialSignature),
  UNIMPLEMENTED_TEST(test_InterceptorCallICFastApi_SimpleSignature),
  UNIMPLEMENTED_TEST(test_InterceptorCallICFastApi_SimpleSignature_Miss1),
  UNIMPLEMENTED_TEST(test_InterceptorCallICFastApi_SimpleSignature_Miss2),
  UNIMPLEMENTED_TEST(test_InterceptorCallICFastApi_SimpleSignature_Miss3),
  UNIMPLEMENTED_TEST(test_InterceptorCallICFastApi_SimpleSignature_TypeError),
  UNIMPLEMENTED_TEST(test_CallICFastApi_TrivialSignature),
  UNIMPLEMENTED_TEST(test_CallICFastApi_SimpleSignature),
  UNIMPLEMENTED_TEST(test_CallICFastApi_SimpleSignature_Miss1),
  UNIMPLEMENTED_TEST(test_CallICFastApi_SimpleSignature_Miss2),
  UNIMPLEMENTED_TEST(test_InterceptorKeyedCallICKeyChange1),
  UNIMPLEMENTED_TEST(test_InterceptorKeyedCallICKeyChange2),
  UNIMPLEMENTED_TEST(test_InterceptorKeyedCallICKeyChangeOnGlobal),
  UNIMPLEMENTED_TEST(test_InterceptorKeyedCallICFromGlobal),
  UNIMPLEMENTED_TEST(test_InterceptorKeyedCallICMapChangeBefore),
  UNIMPLEMENTED_TEST(test_InterceptorKeyedCallICMapChangeAfter),
  UNIMPLEMENTED_TEST(test_InterceptorICReferenceErrors),
  UNIMPLEMENTED_TEST(test_InterceptorICGetterExceptions),
  UNIMPLEMENTED_TEST(test_InterceptorICSetterExceptions),
  UNIMPLEMENTED_TEST(test_NullNamedInterceptor),
  UNIMPLEMENTED_TEST(test_NullIndexedInterceptor),
  UNIMPLEMENTED_TEST(test_NamedPropertyHandlerGetterAttributes),
  UNIMPLEMENTED_TEST(test_Overriding),
  UNIMPLEMENTED_TEST(test_IsConstructCall),
  UNIMPLEMENTED_TEST(test_ObjectProtoToString),
  UNIMPLEMENTED_TEST(test_ObjectGetConstructorName),
  UNIMPLEMENTED_TEST(test_Threading),
  UNIMPLEMENTED_TEST(test_Threading2),
  UNIMPLEMENTED_TEST(test_NestedLockers),
  UNIMPLEMENTED_TEST(test_NestedLockersNoTryCatch),
  UNIMPLEMENTED_TEST(test_RecursiveLocking),
  UNIMPLEMENTED_TEST(test_LockUnlockLock),
  UNIMPLEMENTED_TEST(test_DontLeakGlobalObjects),
  UNIMPLEMENTED_TEST(test_NewPersistentHandleFromWeakCallback),
  UNIMPLEMENTED_TEST(test_DoNotUseDeletedNodesInSecondLevelGc),
  UNIMPLEMENTED_TEST(test_NoGlobalHandlesOrphaningDueToWeakCallback),
  UNIMPLEMENTED_TEST(test_CheckForCrossContextObjectLiterals),
  UNIMPLEMENTED_TEST(test_NestedHandleScopeAndContexts),
  UNIMPLEMENTED_TEST(test_ExternalAllocatedMemory),
  UNIMPLEMENTED_TEST(test_DisposeEnteredContext),
  UNIMPLEMENTED_TEST(test_Regress54),
  UNIMPLEMENTED_TEST(test_CatchStackOverflow),
  UNIMPLEMENTED_TEST(test_TryCatchSourceInfo),
  UNIMPLEMENTED_TEST(test_CompilationCache),
  UNIMPLEMENTED_TEST(test_CallbackFunctionName),
  DISABLED_TEST(test_DateAccess, 20),
  DISABLED_TEST(test_PropertyEnumeration, 22),
  UNIMPLEMENTED_TEST(test_DisableAccessChecksWhileConfiguring),
  UNIMPLEMENTED_TEST(test_AccessChecksReenabledCorrectly),
  UNIMPLEMENTED_TEST(test_AccessControlRepeatedContextCreation),
  UNIMPLEMENTED_TEST(test_TurnOnAccessCheck),
  UNIMPLEMENTED_TEST(test_TurnOnAccessCheckAndRecompile),
  UNIMPLEMENTED_TEST(test_PreCompile),
  UNIMPLEMENTED_TEST(test_PreCompileWithError),
  UNIMPLEMENTED_TEST(test_Regress31661),
  UNIMPLEMENTED_TEST(test_PreCompileSerialization),
  UNIMPLEMENTED_TEST(test_PreCompileDeserializationError),
  UNIMPLEMENTED_TEST(test_PreCompileInvalidPreparseDataError),
  UNIMPLEMENTED_TEST(test_PreCompileAPIVariationsAreSame),
  UNIMPLEMENTED_TEST(test_DictionaryICLoadedFunction),
  UNIMPLEMENTED_TEST(test_CrossContextNew),
  UNIMPLEMENTED_TEST(test_RegExpInterruption),
  UNIMPLEMENTED_TEST(test_ApplyInterruption),
  UNIMPLEMENTED_TEST(test_ObjectClone),
  UNIMPLEMENTED_TEST(test_MorphCompositeStringTest),
  UNIMPLEMENTED_TEST(test_CompileExternalTwoByteSource),
  UNIMPLEMENTED_TEST(test_RegExpStringModification),
  UNIMPLEMENTED_TEST(test_ReadOnlyPropertyInGlobalProto),
  UNIMPLEMENTED_TEST(test_ForceSet),
  UNIMPLEMENTED_TEST(test_ForceSetWithInterceptor),
  UNIMPLEMENTED_TEST(test_ForceDelete),
  UNIMPLEMENTED_TEST(test_ForceDeleteWithInterceptor),
  UNIMPLEMENTED_TEST(test_ForceDeleteIC),
  UNIMPLEMENTED_TEST(test_GetCallingContext),
  UNIMPLEMENTED_TEST(test_InitGlobalVarInProtoChain),
  UNIMPLEMENTED_TEST(test_ReplaceConstantFunction),
  UNIMPLEMENTED_TEST(test_Regress16276),
  UNIMPLEMENTED_TEST(test_PixelArray),
  UNIMPLEMENTED_TEST(test_PixelArrayInfo),
  UNIMPLEMENTED_TEST(test_PixelArrayWithInterceptor),
  UNIMPLEMENTED_TEST(test_ExternalByteArray),
  UNIMPLEMENTED_TEST(test_ExternalUnsignedByteArray),
  UNIMPLEMENTED_TEST(test_ExternalShortArray),
  UNIMPLEMENTED_TEST(test_ExternalUnsignedShortArray),
  UNIMPLEMENTED_TEST(test_ExternalIntArray),
  UNIMPLEMENTED_TEST(test_ExternalUnsignedIntArray),
  UNIMPLEMENTED_TEST(test_ExternalFloatArray),
  UNIMPLEMENTED_TEST(test_ExternalArrays),
  UNIMPLEMENTED_TEST(test_ExternalArrayInfo),
  UNIMPLEMENTED_TEST(test_ScriptContextDependence),
  UNIMPLEMENTED_TEST(test_StackTrace),
  UNIMPLEMENTED_TEST(test_CaptureStackTrace),
  UNIMPLEMENTED_TEST(test_CaptureStackTraceForUncaughtException),
  UNIMPLEMENTED_TEST(test_CaptureStackTraceForUncaughtExceptionAndSetters),
  UNIMPLEMENTED_TEST(test_SourceURLInStackTrace),
  DISABLED_TEST(test_IdleNotification, 24),
  UNIMPLEMENTED_TEST(test_SetResourceConstraints),
  UNIMPLEMENTED_TEST(test_SetResourceConstraintsInThread),
  UNIMPLEMENTED_TEST(test_GetHeapStatistics),
  UNIMPLEMENTED_TEST(test_QuietSignalingNaNs),
  UNIMPLEMENTED_TEST(test_SpaghettiStackReThrow),
  UNIMPLEMENTED_TEST(test_Regress528),
  UNIMPLEMENTED_TEST(test_ScriptOrigin),
  UNIMPLEMENTED_TEST(test_ScriptLineNumber),
  UNIMPLEMENTED_TEST(test_SetterOnConstructorPrototype),
  UNIMPLEMENTED_TEST(test_InterceptorOnConstructorPrototype),
  UNIMPLEMENTED_TEST(test_Bug618),
  UNIMPLEMENTED_TEST(test_GCCallbacks),
  UNIMPLEMENTED_TEST(test_AddToJSFunctionResultCache),
  UNIMPLEMENTED_TEST(test_FillJSFunctionResultCache),
  UNIMPLEMENTED_TEST(test_RoundRobinGetFromCache),
  UNIMPLEMENTED_TEST(test_ReverseGetFromCache),
  UNIMPLEMENTED_TEST(test_TestEviction),
  UNIMPLEMENTED_TEST(test_TwoByteStringInAsciiCons),
  UNIMPLEMENTED_TEST(test_GCInFailedAccessCheckCallback),
  UNIMPLEMENTED_TEST(test_StringCheckMultipleContexts),
  UNIMPLEMENTED_TEST(test_NumberCheckMultipleContexts),
  UNIMPLEMENTED_TEST(test_BooleanCheckMultipleContexts),
  UNIMPLEMENTED_TEST(test_DontDeleteCellLoadIC),
  UNIMPLEMENTED_TEST(test_DontDeleteCellLoadICForceDelete),
  UNIMPLEMENTED_TEST(test_DontDeleteCellLoadICAPI),
  UNIMPLEMENTED_TEST(test_GlobalLoadICGC),
  UNIMPLEMENTED_TEST(test_RegExp),
  TEST(test_Equals),
  UNIMPLEMENTED_TEST(test_NamedEnumeratorAndForIn),
  UNIMPLEMENTED_TEST(test_DefinePropertyPostDetach),
};

const char* file = __FILE__;
#define TEST_NAME "V8 API"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
