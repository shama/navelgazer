/*
Copyright (c) 2013 GitHub Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "handle_map.h"

#include <algorithm>

HandleMap::HandleMap() {
}

HandleMap::~HandleMap() {
  Clear();
}

bool HandleMap::Has(WatcherHandle key) const {
  return map_.find(key) != map_.end();
}

bool HandleMap::Erase(WatcherHandle key) {
  Map::iterator iter = map_.find(key);
  if (iter == map_.end())
    return false;

  NanDisposeUnsafePersistent(iter->second);
  map_.erase(iter);
  return true;
}

void HandleMap::Clear() {
  for (Map::iterator iter = map_.begin(); iter != map_.end(); ++iter)
    NanDisposeUnsafePersistent(iter->second);
  map_.clear();
}

// static
void HandleMap::New(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;
  HandleMap* obj = new HandleMap();
  obj->Wrap(args.This());
}

// static
void HandleMap::Add(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  if (!IsV8ValueWatcherHandle(args[0]))
    return Nan::ThrowTypeError("Bad argument");

  HandleMap* obj = ObjectWrap::Unwrap<HandleMap>(args.This());
  WatcherHandle key = V8ValueToWatcherHandle(args[0]);
  if (obj->Has(key))
    return Nan::ThrowError("Duplicate key");

  NanAssignUnsafePersistent(obj->map_[key], args[1]);
}

// static
void HandleMap::Get(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  if (!IsV8ValueWatcherHandle(args[0]))
    return Nan::ThrowTypeError("Bad argument");

  HandleMap* obj = ObjectWrap::Unwrap<HandleMap>(args.This());
  WatcherHandle key = V8ValueToWatcherHandle(args[0]);
  if (!obj->Has(key))
    return Nan::ThrowError("Invalid key");

  args.GetReturnValue().Set(NanUnsafePersistentToLocal(obj->map_[key]));
}

// static
void HandleMap::Has(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  if (!IsV8ValueWatcherHandle(args[0]))
    return Nan::ThrowTypeError("Bad argument");

  HandleMap* obj = ObjectWrap::Unwrap<HandleMap>(args.This());
  args.GetReturnValue().Set(Nan::New<Boolean>(obj->Has(V8ValueToWatcherHandle(args[0]))));
}

// static
void HandleMap::Values(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  HandleMap* obj = ObjectWrap::Unwrap<HandleMap>(args.This());

  int i = 0;
  Local<Array> keys = Nan::New<Array>(obj->map_.size());
  for (Map::const_iterator iter = obj->map_.begin();
       iter != obj->map_.end();
       ++iter, ++i)
    keys->Set(i, NanUnsafePersistentToLocal(iter->second));

  args.GetReturnValue().Set(keys);
}

// static
void HandleMap::Remove(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  if (!IsV8ValueWatcherHandle(args[0]))
    return Nan::ThrowTypeError("Bad argument");

  HandleMap* obj = ObjectWrap::Unwrap<HandleMap>(args.This());
  if (!obj->Erase(V8ValueToWatcherHandle(args[0])))
    return Nan::ThrowError("Invalid key");
}

// static
void HandleMap::Clear(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  HandleMap* obj = ObjectWrap::Unwrap<HandleMap>(args.This());
  obj->Clear();
}

// static
void HandleMap::Initialize(Local<Object> target) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(HandleMap::New);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(Nan::New("HandleMap").ToLocalChecked());

  Nan::SetPrototypeMethod(t, "add", Add);
  Nan::SetPrototypeMethod(t, "get", Get);
  Nan::SetPrototypeMethod(t, "has", Has);
  Nan::SetPrototypeMethod(t, "values", Values);
  Nan::SetPrototypeMethod(t, "remove", Remove);
  Nan::SetPrototypeMethod(t, "clear", Clear);

  target->Set(Nan::New("HandleMap").ToLocalChecked(), t->GetFunction());
}
