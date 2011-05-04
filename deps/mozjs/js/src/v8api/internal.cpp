#include "v8-internal.h"

namespace v8 {
namespace internal {

////////////////////////////////////////////////////////////////////////////////
//// Accessor Storage

AccessorStorage::AccessorStorage()
{
  mStore.init(10);
}

void
AccessorStorage::addAccessor(jsid name,
                             AccessorGetter getter,
                             AccessorSetter setter,
                             Handle<Value> data,
                             PropertyAttribute attribute)
{
  JS_ASSERT(mStore.initialized());

  PropertyData container = {
    getter,
    setter,
    Persistent<Value>::New(data),
    attribute,
  };
  AccessorTable::AddPtr slot = mStore.lookupForAdd(name);
  if (slot.found()) {
    mStore.remove(slot);
    slot = mStore.lookupForAdd(name);
  }
  mStore.add(slot, name, container);
}

AccessorStorage::PropertyData
AccessorStorage::get(jsid name) const
{
  JS_ASSERT(mStore.initialized());

  AccessorTable::Ptr p = mStore.lookup(name);
  JS_ASSERT(p);
  return p->value;
}

AccessorStorage::Range
AccessorStorage::all() const
{
  JS_ASSERT(mStore.initialized());

  return mStore.all();
}

////////////////////////////////////////////////////////////////////////////////
//// Attribute Storage

AttributeStorage::AttributeStorage()
{
  mStore.init(10);
}

void
AttributeStorage::addAttribute(jsid name,
                               Handle<Value> data)
{
  JS_ASSERT(mStore.initialized());

  AttributeTable::AddPtr slot = mStore.lookupForAdd(name);
  if (slot.found()) {
    mStore.remove(slot);
    slot = mStore.lookupForAdd(name);
  }
  mStore.add(slot, name, Persistent<Value>::New(data));
}

AttributeStorage::Range
AttributeStorage::all() const
{
  JS_ASSERT(mStore.initialized());

  return mStore.all();
}

////////////////////////////////////////////////////////////////////////////////
//// Debug Helpers

#ifdef DEBUG
void
Dump(Handle<Object> obj)
{
  js_DumpObject(**obj);
}

void
Dump(Object& obj)
{
  Local<Object> o = Local<Object>::New(&obj);
  Dump(o);
}

void
Dump(Handle<String> str)
{
  jsval v = str->native();
  js_DumpString(JSVAL_TO_STRING(v));
}

void
Dump(String& str)
{
  Local<String> s = Local<String>::New(&str);
  Dump(s);
}
#endif // DEBUG

} // namespace internal
} // namespace v8