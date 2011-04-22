#include "v8-internal.h"

namespace v8 { namespace internal {

const int KB = 1024;
const int MB = 1024 * 1024;
static JSRuntime *gRuntime = 0;
JSRuntime *rt() {
  if (!gRuntime) {
    V8::Initialize();
  }
  return gRuntime;
}

static JSContext *gRootContext = 0;
JSContext *cx() {
  if (!gRootContext) {
    V8::Initialize();
  }
  return gRootContext;
}

JSClass global_class = {
  "global", JSCLASS_GLOBAL_FLAGS,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  JSCLASS_NO_OPTIONAL_MEMBERS
};

void notImplemented(const char* functionName) {
  fprintf(stderr, "Calling an unimplemented API: %s\n", functionName);
}

static JSObject* gCompartment = 0;
static JSCrossCompartmentCall *gCompartmentCall = 0;
static bool gHasAttemptedInitialization = false;
static FatalErrorCallback gFatalCallback = 0;

}

using namespace internal;

bool V8::Initialize() {
  if (gHasAttemptedInitialization)
    return true;
  gHasAttemptedInitialization = true;
  JS_SetCStringsAreUTF8();
  JS_ASSERT(!gRuntime && !gRootContext && !gCompartment && !gCompartment && !gCompartmentCall);
  gRuntime = JS_NewRuntime(64 * MB);
  if(!gRuntime)
    return false;

  JSContext *ctx(JS_NewContext(gRuntime, 8192));
  if (!ctx)
    return false;
  // TODO: look into JSOPTION_NO_SCRIPT_RVAL
  JS_SetOptions(ctx, JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT | JSOPTION_DONT_REPORT_UNCAUGHT);
  JS_SetVersion(ctx, JSVERSION_LATEST);
  JS_SetErrorReporter(ctx, TryCatch::ReportError);

  JS_BeginRequest(ctx);

  gRootContext = ctx;

  gCompartment = JS_NewCompartmentAndGlobalObject(ctx, &global_class, NULL);
  if (!gCompartment)
    return false;
  gCompartmentCall = JS_EnterCrossCompartmentCall(ctx, gCompartment);
  if (!gCompartmentCall)
    return false;

  (void) JS_AddObjectRoot(ctx, &gCompartment);

  JS_SetGCCallback(cx(), GCCallback);
  return true;
}

// TODO: call this
bool V8::Dispose() {
  JS_LeaveCrossCompartmentCall(gCompartmentCall);
  (void) JS_RemoveObjectRoot(gRootContext, &gCompartment);
  gCompartment = 0;
  JS_EndRequest(gRootContext);
  gRootContext = 0;
  if (gRuntime)
    JS_DestroyRuntime(gRuntime);
  gRuntime = 0;
  JS_ShutDown();
  return true;
}

Handle<Value> ThrowException(Handle<Value> exception) {
  jsval native = exception->native();
  JS_SetPendingException(cx(), native);
  JS_ReportPendingException(cx());

  return Undefined();
}

bool V8::IdleNotification() {
  // XXX Returning true here will tell Node to stop running the GC timer, so
  //     we're just going to do that for now.
  return true;
}

void V8::GetHeapStatistics(HeapStatistics* aHeapStatistics) {
  UNIMPLEMENTEDAPI();
}

const char* V8::GetVersion() {
  JSVersion version = JS_GetVersion(cx());
  return JS_VersionToString(version);
}

void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool aRemoveFlags) {
}

void V8::SetFatalErrorHandler(FatalErrorCallback aCallback) {
  gFatalCallback = aCallback;
}

int V8::AdjustAmountOfExternalAllocatedMemory(int aChangeInBytes) {
  static int externalMemory = 0;
  return externalMemory += aChangeInBytes;
}

void V8::AddGCPrologueCallback(GCPrologueCallback aCallback, GCType aGCTypeFilter) {
  UNIMPLEMENTEDAPI();
}

void V8::LowMemoryNotification() {
  UNIMPLEMENTEDAPI();
}

JSBool V8::GCCallback(JSContext *cx, JSGCStatus status) {
  if (status == JSGC_MARK_END) {
    PersistentGCReference::CheckForWeakHandles();
  }
  // TODO: what do I do here?
  return JS_FALSE;
}

void V8::ReportError(JSContext *ctx, const char *message, JSErrorReport *report) {
  if (gFatalCallback) {
    // TODO: figure this out
    bool isFatal = false;
    if (isFatal) {
      // TODO: better location reporting?
      gFatalCallback(report->filename, message);
    }
  }
  TryCatch::ReportError(ctx, message, report);
}

static Local<Value> ConstructError(const char *name, Handle<String> message) {
  // XXX: this probably isn't correct in all cases
  Handle<Context> ctx = Context::GetCurrent();
  Handle<Object> global = ctx->Global();
  Handle<Value> ctor = global->Get(String::New(name));
  if (ctor.IsEmpty() || !ctor->IsFunction())
    return Local<Value>();
  Handle<Function> fn = ctor.As<Function>();
  Handle<Value> args[] = { Handle<Value>(message) };
  return Local<Value>::New(fn->NewInstance(1, args));
}

Local<Value> Exception::RangeError(Handle<String> message) {
  return ConstructError("RangeError", message);
}

Local<Value> Exception::ReferenceError(Handle<String> message) {
  return ConstructError("ReferenceError", message);
}

Local<Value> Exception::SyntaxError(Handle<String> message) {
  return ConstructError("SyntaxError", message);
}

Local<Value> Exception::TypeError(Handle<String> message) {
  return ConstructError("TypeError", message);
}

Local<Value> Exception::Error(Handle<String> message) {
  return ConstructError("Error", message);
}

}
