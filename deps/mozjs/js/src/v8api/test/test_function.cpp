/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

#include "v8api_test_harness.h"

////////////////////////////////////////////////////////////////////////////////
//// Tests

Handle<Value> AddOne(const Arguments& args) {
  do_check_eq(args.Length(), 1);
  Local<Value> v = args[0];
  do_check_true(v->IsNumber());
  Local<Number> n = v->ToNumber();
  return Number::New(n->Value() + 1.0);
}

void
test_BasicCall()
{
  HandleScope handle_scope;
  Handle<ObjectTemplate> templ = ObjectTemplate::New();
  Handle<FunctionTemplate> fnT = v8::FunctionTemplate::New(AddOne);
  templ->Set("AddOne", fnT);

  Persistent<Context> context = Context::New(NULL, templ);
  Context::Scope context_scope(context);
  Local<Value> addone = context->Global()->Get(String::New("AddOne"));
  do_check_true(!addone.IsEmpty());
  do_check_true(!addone->IsUndefined());
  do_check_true(addone->IsObject());
  do_check_true(addone->IsFunction());
  Local<Function> fn = addone.As<Function>();
  do_check_eq(fn, fnT->GetFunction());
  Local<Number> n = Number::New(0.5);
  Handle<Value> args[] = { n };
  Local<Value> v = fn->Call(context->Global(), 1, args);
  do_check_true(!v.IsEmpty());
  do_check_true(v->IsNumber());
  Local<Number> n2 = v->ToNumber();
  do_check_eq(n2->Value(), 1.5);
}

Handle<Value> InvokeCallback(const Arguments& args) {
  return v8::Undefined();
}

Handle<Value> InstanceAccessorCallback(Local<String> property, const AccessorInfo &info) {
  return v8::Undefined();
}

void
test_V8DocExample()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New();
  t->Set("func_property", v8::Number::New(1));

  v8::Local<v8::Template> proto_t = t->PrototypeTemplate();
  proto_t->Set("proto_method", v8::FunctionTemplate::New(InvokeCallback));
  proto_t->Set("proto_const", v8::Number::New(2));

  v8::Local<v8::ObjectTemplate> instance_t = t->InstanceTemplate();
  instance_t->SetAccessor(String::New("instance_accessor"), InstanceAccessorCallback);
  instance_t->Set("instance_property", Number::New(3));

  v8::Local<v8::Function> function = t->GetFunction();
  v8::Local<v8::Object> instance = function->NewInstance();
  // TODO: test that we created the objects we were supposed to
}

void
test_Constructor()
{
  HandleScope handle_scope;
  Handle<ObjectTemplate> templ = ObjectTemplate::New();
  Handle<FunctionTemplate> fnT = v8::FunctionTemplate::New(AddOne);
  templ->Set("AddOne", fnT);

  Persistent<Context> context = Context::New(NULL, templ);
  Handle<Script> script = Script::New(String::New("new AddOne(4);"));

  Context::Scope scope(context);
  script->Run();
}

void
test_Name()
{
  HandleScope handle_scope;
  Handle<ObjectTemplate> templ = ObjectTemplate::New();
  Handle<FunctionTemplate> fnT = v8::FunctionTemplate::New(AddOne);
  templ->Set("AddOne", fnT);
  fnT->SetClassName(String::NewSymbol("AddOne"));

  Persistent<Context> context = Context::New(NULL, templ);
  Handle<Script> script = Script::New(String::New("AddOne.name;"));

  Context::Scope scope(context);
  Handle<Value> v = script->Run();
  do_check_true(!v.IsEmpty());
  Handle<String> s = v->ToString();
  do_check_eq(s, String::NewSymbol("AddOne"));
}

////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_BasicCall),
  TEST(test_V8DocExample),
  TEST(test_Constructor),
  TEST(test_Name),
};

const char* file = __FILE__;
#define TEST_NAME "FunctionTemplate Class"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"

