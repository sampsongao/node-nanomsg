#include "bus.h"
#include "inproc.h"
#include "ipc.h"
#include "nn.h"
#include "pair.h"
#include "pipeline.h"
#include "poll_ctx.h"
#include "pubsub.h"
#include "reqrep.h"
#include "survey.h"
#include "tcp.h"
#include "ws.h"

using Napi::Function;
using Napi::FunctionReference;
using Napi::Number;
using Napi::Object;
using Napi::String;
using Napi::Value;

Napi::Value Socket(const Napi::CallbackInfo& info) {
  int domain = info[0].As<Napi::Number>().Int32Value();
  int protocol = info[1].As<Napi::Number>().Int32Value();

  return Napi::Number::New(info.Env(), nn_socket(domain, protocol));
}

Napi::Value Close(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int rc = 0;

  do {
    rc = nn_close(s);
  } while (rc < 0 && errno == EINTR);

  return Napi::Number::New(info.Env(), rc);
}

Napi::Value Setopt(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int level = info[1].As<Napi::Number>().Int32Value();
  int option = info[2].As<Napi::Number>().Int32Value();
  int optval = info[3].As<Napi::Number>().Int32Value();

  return Napi::Number::New(info.Env(), 
      nn_setsockopt(s, level, option, &optval, sizeof(optval)));
}

Napi::Value Getopt(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int level = info[1].As<Napi::Number>().Int32Value();
  int option = info[2].As<Napi::Number>().Int32Value();
  int optval;
  size_t optsize = sizeof(optval);

  // check if the function succeeds
  if (nn_getsockopt(s, level, option, &optval, &optsize) == 0) {
    return Napi::Number::New(info.Env(), optval);
  }
}

Napi::Value Chan(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int level = NN_SUB;
  int option = info[1].As<Napi::Number>().Int32Value();
  Napi::String str(info.Env(), info[2]);

  return 
      Napi::Number::New(info.Env(), nn_setsockopt(s, level, option, *str, str.Length()));
}

Napi::Value Bind(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  Napi::String addr(info.Env(), info[1]);

  return Napi::Number::New(info.Env(), nn_bind(s, *addr));
}

Napi::Value Connect(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  Napi::String addr(info.Env(), info[1]);

  return Napi::Number::New(info.Env(), nn_connect(s, *addr));
}

Napi::Value Shutdown(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int how = info[1].As<Napi::Number>().Int32Value();

  return Napi::Number::New(info.Env(), nn_shutdown(s, how));
}

Napi::Value Send(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int flags = info[2].As<Napi::Number>().Int32Value();

  if (info[1].IsBuffer()) {
    return Napi::Number::New(info.Env(), nn_send(
        s, info[1].As<Napi::Buffer<char>>().Data(), info[1].As<Napi::Buffer<char>>().Length(), flags));
  } else {
    Napi::String str(info.Env(), info[1]);
    return 
        Napi::Number::New(info.Env(), nn_send(s, *str, str.Length(), flags));
  }
}

static void fcb(napi_env env, void* data, void *) {
  nn_freemsg(data);
}

Napi::Value Recv(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int flags = info[1].As<Napi::Number>().Int32Value();

  // Invoke nanomsg function.
  char *buf = NULL;
  int len = nn_recv(s, &buf, NN_MSG, flags);

  if (len > 0) {
    Napi::Object h = Napi::Buffer<char>::New(info.Env(), buf, static_cast<size_t>(len), fcb, 0);
    return h;
  } else {
    return Napi::Number::New(info.Env(), len);
  }
}

Napi::Value SymbolInfo(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  struct nn_symbol_properties prop;
  int ret = nn_symbol_info(s, &prop, sizeof(prop));

  if (ret > 0) {
    Napi::Object obj = Napi::Object::New(info.Env());
    obj.Set(Napi::String::New(info.Env(), "value"), Napi::Number::New(info.Env(), prop.value));
    obj.Set(Napi::String::New(info.Env(), "ns"), Napi::Number::New(info.Env(), prop.ns));
    obj.Set(Napi::String::New(info.Env(), "type"), Napi::Number::New(info.Env(), prop.type));
    obj.Set(Napi::String::New(info.Env(), "unit"), Napi::Number::New(info.Env(), prop.unit));
    obj.Set(Napi::String::New(info.Env(), "name"), Napi::String::New(info.Env(), prop.name));
    return obj;
  } else if (ret != 0) {
    Napi::Error::New(info.Env(), nn_strerror(nn_errno())).ThrowAsJavaScriptException();
  }
}

Napi::Value Symbol(const Napi::CallbackInfo& info) {
  int s = info[0].As<Napi::Number>().Int32Value();
  int val;
  const char *ret = nn_symbol(s, &val);

  if (ret) {
    Napi::Object obj = Napi::Object::New(info.Env());
    obj.Set(Napi::String::New(info.Env(), "value"), Napi::Number::New(info.Env(), val));
    obj.Set(Napi::String::New(info.Env(), "name"), Napi::String::New(info.Env(), ret));
    return obj;
  } else {
    // symbol index out of range
    // this behaviour seems inconsistent with SymbolInfo() above
    // but we are faithfully following the libnanomsg API, warta and all
    Napi::Error::New(info.Env(), nn_strerror(nn_errno())).ThrowAsJavaScriptException(); // EINVAL
  }
}

void Term(const Napi::CallbackInfo& info) { nn_term(); }

// Pass in two sockets, or (socket, -1) or (-1, socket) for loopback
void Device(const Napi::CallbackInfo& info) {
  int s1 = info[0].As<Napi::Number>().Int32Value();
  int s2 = info[1].As<Napi::Number>().Int32Value();

  // nn_device only returns when it encounters an error
  nn_device(s1, s2);
  Napi::Error::New(info.Env(), nn_strerror(nn_errno())).ThrowAsJavaScriptException();
}

Napi::Value Errno(const Napi::CallbackInfo& info) { return Napi::Number::New(info.Env(), nn_errno()); }

Napi::Value Err(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), nn_strerror(nn_errno()));
}

Napi::Value PollSocket(const Napi::CallbackInfo& info) {
  const int s = info[0].As<Napi::Number>().Int32Value();
  const bool is_sender = info[1].As<Napi::Boolean>().Value();
  const Napi::Function cb = info[2].As<Function>();
  PollCtx *context = new PollCtx(s, is_sender, cb);
  return PollCtx::WrapPointer(info.Env(), context, sizeof context);
}

static void close_cb(uv_handle_t *handle) {
  const PollCtx* const context = static_cast<PollCtx*>(handle->data);
  delete context;
}

void PollStop(const Napi::CallbackInfo& info) {
  PollCtx* const context = PollCtx::UnwrapPointer(info[0]);
  if (context != NULL) {
    uv_close(reinterpret_cast<uv_handle_t*>(&context->poll_handle), close_cb);
  }
  // TODO: the else case should never happen.  Maybe add an assert or
  // something.
}

class NanomsgDeviceWorker : public Napi::AsyncWorker {
public:
  NanomsgDeviceWorker(Napi::Env env, Napi::Function callback, int s1, int s2)
      : Napi::AsyncWorker(callback), s1(s1), s2(s2) {}
  ~NanomsgDeviceWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute() {
    // nn_errno() only returns on error
    nn_device(s1, s2);
    err = nn_errno();
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void OnOK() {
    Napi::HandleScope scope(Env());

    _callback.MakeCallback({ Napi::Number::New(Env(), err) });
  };

private:
  int s1;
  int s2;
  int err;
};

// Asynchronous access to the `nn_device()` function
void DeviceWorker(const Napi::CallbackInfo& info) {
  int s1 = info[0].As<Napi::Number>().Int32Value();
  int s2 = info[1].As<Napi::Number>().Int32Value();
  Napi::Function *callback = new Napi::Function(info[2].As<Function>());

  NanomsgDeviceWorker* worker = new NanomsgDeviceWorker(info.Env(), *callback, s1, s2);
  worker->Queue();
}

#define EXPORT_METHOD(C, S)                                            \
  C.Set(Napi::String::New(env, #S),                           \
    Napi::Function::New(env, S));

void InitAll(Napi::Env env, Napi::Object exports, Napi::Object module) {
  Napi::HandleScope scope(env);

  // Export functions.
  EXPORT_METHOD(exports, Socket);
  EXPORT_METHOD(exports, Close);
  EXPORT_METHOD(exports, Chan);
  EXPORT_METHOD(exports, Bind);
  EXPORT_METHOD(exports, Connect);
  EXPORT_METHOD(exports, Shutdown);
  EXPORT_METHOD(exports, Send);
  EXPORT_METHOD(exports, Recv);
  EXPORT_METHOD(exports, Errno);
  EXPORT_METHOD(exports, PollSocket);
  EXPORT_METHOD(exports, PollStop);
  EXPORT_METHOD(exports, DeviceWorker);
  EXPORT_METHOD(exports, SymbolInfo);
  EXPORT_METHOD(exports, Symbol);
  EXPORT_METHOD(exports, Term);

  EXPORT_METHOD(exports, Getopt);
  EXPORT_METHOD(exports, Setopt);
  EXPORT_METHOD(exports, Err);

  // Export symbols.
  for (int i = 0;; ++i) {
    int value;
    const char *symbol_name = nn_symbol(i, &value);
    if (symbol_name == NULL) {
      break;
    }
    exports.Set(Napi::String::New(env, symbol_name), Napi::Number::New(env, value));
  }
}

NODE_API_MODULE(node_nanomsg, InitAll)
