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
#include "common.h"

static uv_async_t g_async;
static int g_watch_count;
static uv_sem_t g_semaphore;
static uv_thread_t g_thread;

static EVENT_TYPE g_type;
static WatcherHandle g_handle;
static std::vector<char> g_new_path;
static std::vector<char> g_old_path;
static Nan::Persistent<Function> g_callback;

static void CommonThread(void* handle) {
  WaitForMainThread();
  PlatformThread();
}

#if NODE_VERSION_AT_LEAST(0, 11, 13)
static void MakeCallbackInMainThread(uv_async_t* handle) {
#else
static void MakeCallbackInMainThread(uv_async_t* handle, int status) {
#endif
  Nan::HandleScope scope;

  if (!g_callback.IsEmpty()) {
    Local<String> type;
    switch (g_type) {
      case EVENT_CHANGE:
        type = Nan::New("change").ToLocalChecked();
        break;
      case EVENT_DELETE:
        type = Nan::New("delete").ToLocalChecked();
        break;
      case EVENT_RENAME:
        type = Nan::New("rename").ToLocalChecked();
        break;
      case EVENT_CHILD_CREATE:
        type = Nan::New("child-create").ToLocalChecked();
        break;
      case EVENT_CHILD_CHANGE:
        type = Nan::New("child-change").ToLocalChecked();
        break;
      case EVENT_CHILD_DELETE:
        type = Nan::New("child-delete").ToLocalChecked();
        break;
      case EVENT_CHILD_RENAME:
        type = Nan::New("child-rename").ToLocalChecked();
        break;
      default:
        type = Nan::New("unknown").ToLocalChecked();
        return;
    }

    Local<Value> argv[] = {
        type,
        WatcherHandleToV8Value(g_handle),
        Nan::New(std::string(g_new_path.begin(), g_new_path.end())).ToLocalChecked(),
        Nan::New(std::string(g_old_path.begin(), g_old_path.end())).ToLocalChecked(),
    };
    Nan::New(g_callback)->Call(Nan::GetCurrentContext()->Global(), 4, argv);
  }

  WakeupNewThread();
}

static void SetRef(bool value) {
  uv_handle_t* h = reinterpret_cast<uv_handle_t*>(&g_async);
  if (value) {
    uv_ref(h);
  } else {
    uv_unref(h);
  }
}

void CommonInit() {
  uv_sem_init(&g_semaphore, 0);
  uv_async_init(uv_default_loop(), &g_async, MakeCallbackInMainThread);
  // As long as any uv_ref'd uv_async_t handle remains active, the node
  // process will never exit, so we must call uv_unref here (#47).
  SetRef(false);
  g_watch_count = 0;
  uv_thread_create(&g_thread, &CommonThread, NULL);
}

void WaitForMainThread() {
  uv_sem_wait(&g_semaphore);
}

void WakeupNewThread() {
  uv_sem_post(&g_semaphore);
}

void PostEventAndWait(EVENT_TYPE type,
                      WatcherHandle handle,
                      const std::vector<char>& new_path,
                      const std::vector<char>& old_path) {
  // FIXME should not pass args by settings globals.
  g_type = type;
  g_handle = handle;
  g_new_path = new_path;
  g_old_path = old_path;

  uv_async_send(&g_async);
  WaitForMainThread();
}

void SetCallback(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  if (!args[0]->IsFunction())
    return Nan::ThrowTypeError("Function required");

  g_callback.Reset(Local<Function>::Cast(args[0]));
}

void Watch(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  if (!args[0]->IsString())
    return Nan::ThrowTypeError("String required");

  Local<String> path = args[0]->ToString();
  WatcherHandle handle = PlatformWatch(*String::Utf8Value(path));
  if (PlatformIsEMFILE(handle))
    return Nan::ThrowTypeError("EMFILE: Unable to watch path");
  if (!PlatformIsHandleValid(handle))
    return Nan::ThrowTypeError("Unable to watch path");

  if (g_watch_count++ == 0) {
    SetRef(true);
  }

  args.GetReturnValue().Set(WatcherHandleToV8Value(handle));
}

void Unwatch(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::HandleScope scope;

  if (!IsV8ValueWatcherHandle(args[0]))
    return Nan::ThrowTypeError("Handle type required");

  PlatformUnwatch(V8ValueToWatcherHandle(args[0]));

  if (--g_watch_count == 0) {
    SetRef(false);
  }
}
