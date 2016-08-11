#include <stdio.h>
#include <stdlib.h>
#include "node_pointer.h"

#include <nn.h>

#include <inproc.h>
#include <ipc.h>
#include <tcp.h>
#include <ws.h>

#include <pubsub.h>
#include <pipeline.h>
#include <bus.h>
#include <pair.h>
#include <reqrep.h>
#include <survey.h>

#include <node_jsvmapi.h>

// NAN_METHOD(Socket) {
//   int domain = Nan::To<int>(info[0]).FromJust();
//   int protocol = Nan::To<int>(info[1]).FromJust();

//   info.GetReturnValue().Set(Nan::New<Number>(nn_socket(domain, protocol)));
// }

void Socket(napi_env env, napi_func_cb_info info) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int domain = napi_get_value_int64(env, args[0]);
  int protocol = napi_get_value_int64(env, args[1]);
  napi_value ret = napi_create_number(env, nn_socket(domain, protocol));
  napi_set_return_value(env, info, ret);
}

// NAN_METHOD(Close) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int rc = 0;

//   do {
//     rc = nn_close(s);
//   } while (rc < 0 && errno == EINTR);

//   info.GetReturnValue().Set(Nan::New<Number>(rc));
// }

void Close(napi_env env, napi_func_cb_info info) {
  napi_value args[1];
  napi_get_cb_args(env, info, args, 1);

  int s = napi_get_value_int64(env, args[0]);
  int rc = 0;

  do {
    rc = nn_close(s);
  } while (rc < 0 && errno == EINTR);

  napi_value ret = napi_create_number(env, rc);
  napi_set_return_value(env, info, ret);
}

// NAN_METHOD(Setopt) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int level = Nan::To<int>(info[1]).FromJust();
//   int option = Nan::To<int>(info[2]).FromJust();
//   int optval = Nan::To<int>(info[3]).FromJust();

//   info.GetReturnValue().Set(Nan::New<Number>(
//       nn_setsockopt(s, level, option, &optval, sizeof(optval))));
// }

void Setopt(napi_env env, napi_func_cb_info info) {
  napi_value args[4];
  napi_get_cb_args(env, info, args, 4);

  int s = napi_get_value_int64(env, args[0]);
  int level = napi_get_value_int64(env, args[1]);
  int option = napi_get_value_int64(env, args[2]);
  int optval = napi_get_value_int64(env, args[3]);
  
  int result = nn_setsockopt(s, level, option, &optval, sizeof(optval));
  napi_value ret = napi_create_number(env, result);
  napi_set_return_value(env, info, ret);
}

// NAN_METHOD(Getopt) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int level = Nan::To<int>(info[1]).FromJust();
//   int option = Nan::To<int>(info[2]).FromJust();
//   int optval;
//   size_t optsize = sizeof(optval);

//   // check if the function succeeds
//   if (nn_getsockopt(s, level, option, &optval, &optsize) == 0) {
//     info.GetReturnValue().Set(Nan::New<Number>(optval));
//   }
// }

void Getopt(napi_env env, napi_func_cb_info info) {
  napi_value args[3];
  napi_get_cb_args(env, info, args, 3);

  int s = napi_get_value_int64(env, args[0]);
  int level = napi_get_value_int64(env, args[1]);
  int option = napi_get_value_int64(env, args[2]);
  int optval;
  size_t optsize = sizeof(optval);

  // check if the function succeeds
  if (nn_getsockopt(s, level, option, &optval, &optsize) == 0) {
    napi_value ret = napi_create_number(env, optval);
    napi_set_return_value(env, info, optval);
  }
}

// NAN_METHOD(Chan) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int level = NN_SUB;
//   int option = Nan::To<int>(info[1]).FromJust();
//   v8::String::Utf8Value str(info[2]);

//   info.GetReturnValue().Set(
//       Nan::New<Number>(nn_setsockopt(s, level, option, *str, str.length())));
// }

void Chan(napi_env env, napi_func_cb_info info) {
  napi_value args[3];
  napi_get_cb_args(env, info, args, 3);

  int s = napi_get_value_int64(env, args[0]);
  int level = NN_SUB;
  int option = napi_get_value_int64(env, args[1]);
  char str[1024];
  int remain = napi_get_string_from_value(env, args[2], str, 1024);
  int length = napi_get_string_utf8_length(env, args[2]);
  
  int result = nn_setsockopt(s, level, option, str, length);
  napi_value ret = napi_create_number(env, result);
  napi_set_return_value(env, info, ret);
}

// NAN_METHOD(Bind) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   String::Utf8Value addr(info[1]);

//   info.GetReturnValue().Set(Nan::New<Number>(nn_bind(s, *addr)));
// }

void Bind(napi_env env, napi_func_cb_info info) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  char addr[1024];
  int remain = napi_get_string_from_value(env, args[1], addr, 1024);
  int length = napi_get_string_utf8_length(env, args[1]);

  napi_value ret = napi_create_number(env, nn_bind(s, addr));
  napi_set_return_value(env, ret);
}

// NAN_METHOD(Connect) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   String::Utf8Value addr(info[1]);

//   info.GetReturnValue().Set(Nan::New<Number>(nn_connect(s, *addr)));
// }

void Connect(napi_env env, napi_func_cb_info info) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  char addr[1024];
  int remain = napi_get_string_from_value(env, args[1], addr, 1024);
  int length = napi_get_string_utf8_length(env, args[1]);

  napi_value ret = napi_create_number(env, nn_connect(s, addr));
  napi_set_return_value(env, ret);
}

// NAN_METHOD(Shutdown) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int how = Nan::To<int>(info[1]).FromJust();

//   info.GetReturnValue().Set(Nan::New<Number>(nn_shutdown(s, how)));
// }

void Shutdown(napi_env env, napi_func_cb_info info) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  int how = napi_get_value_int64(env, args[1]);

  napi_value ret = napi_create_number(env, nn_shutdown(s, addr));
  napi_set_return_value(env, ret);
}

// NAN_METHOD(Send) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int flags = Nan::To<int>(info[2]).FromJust();

//   if (node::Buffer::HasInstance(info[1])) {
//     info.GetReturnValue().Set(Nan::New<Number>(nn_send(
//         s, node::Buffer::Data(info[1]), node::Buffer::Length(info[1]), flags)));
//   } else {
//     v8::String::Utf8Value str(info[1]);
//     info.GetReturnValue().Set(
//         Nan::New<Number>(nn_send(s, *str, str.length(), flags)));
//   }
// }

void Send(napi_env env, napi_func_cb_info info) {
  napi_value args[3];
  napi_get_cb_args(env, info, args, 3);

  int s = napi_get_value_int64(env, args[0]);
  int flags = napi_get_value_int64(env, args[2]);

  if (napi_buffer_has_instance(info[1])) {
    int result = nn_send(s, napi_buffer_data(args[1], napi_buffer_length(args[1]), flags);
    napi_value ret = napi_create_number(env, result);
    napi_set_return_value(env, ret);
  } else {
    char str[1024];
    int remain = napi_get_string_from_value(env, args[1], str, 1024);
    int length = napi_get_string_utf8_length(env, args[1]);
    int result = nn_send(s, str, length, flags);
    napi_value ret = napi_create_number(env, result);
    napi_set_return_value(env, ret);
  }
}

// void fcb(char *data, void *hint) {
//   nn_freemsg(data);
//   (void) hint;
// }

// NAN_METHOD(Recv) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int flags = Nan::To<int>(info[1]).FromJust();

//   // Invoke nanomsg function.
//   char *buf = NULL;
//   int len = nn_recv(s, &buf, NN_MSG, flags);

//   if (len > -1) {
//     v8::Local<v8::Object> h = Nan::NewBuffer(buf, len, fcb, 0).ToLocalChecked();
//     info.GetReturnValue().Set(h);
//   } else {
//     info.GetReturnValue().Set(Nan::New<Number>(len));
//   }
// }

void Recv(napi_env env, napi_func_cb_info info) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  int flags = napi_get_value_int64(env, args[1]);

  char* buf = NULL;
  int len = nn_recv(s, &buf, NN_MSG, flags);

  if (len > -1) {
    napi_value h = napi_buffer_copy(env, buf, len);
    napi_set_return_value(env, h);
  } else {
    napi_set_return_value(env, napi_create_number(env, len));
  }
}

// NAN_METHOD(SymbolInfo) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   struct nn_symbol_properties prop;
//   int ret = nn_symbol_info(s, &prop, sizeof(prop));

//   if (ret > 0) {
//     Local<Object> obj = Nan::New<Object>();
//     Nan::Set(obj, Nan::New("value").ToLocalChecked(),
//              Nan::New<Number>(prop.value));
//     Nan::Set(obj, Nan::New("ns").ToLocalChecked(), Nan::New<Number>(prop.ns));
//     Nan::Set(obj, Nan::New("type").ToLocalChecked(),
//              Nan::New<Number>(prop.type));
//     Nan::Set(obj, Nan::New("unit").ToLocalChecked(),
//              Nan::New<Number>(prop.unit));
//     Nan::Set(obj, Nan::New("name").ToLocalChecked(),
//              Nan::New<String>(prop.name).ToLocalChecked());
//     info.GetReturnValue().Set(obj);
//   } else if (ret != 0) {
//     Nan::ThrowError(nn_strerror(nn_errno()));
//   }
// }

void SymbolInfo(napi_env env, napi_func_cb_info info) {
  napi_value args[1];
  napi_get_cb_args(env, info, args, 1);

  int s = napi_get_value_int64(env, args[0]);

  struct nn_symbol_properties prop;
  int ret = nn_symbol_info(s, &prop, sizeof(prop));

  if (ret > 0) {
    napi_value obj = napi_create_object(env);
    napi_set_property(env, obj, napi_property_name("value"), 
                      napi_create_number(prop.value));
    napi_set_property(env, obj, napi_property_name("ns"), 
                      napi_create_number(prop.ns));
    napi_set_property(env, obj, napi_property_name("type"), 
                      napi_create_number(prop.type));
    napi_set_property(env, obj, napi_property_name("unit"), 
                      napi_create_number(prop.unit));
    napi_set_property(env, obj, napi_property_name("name"), 
                      napi_create_number(prop.name));

    napi_set_return_value(env, obj);
  } else if (ret != 0) {
    napi_throw_error(env, nn_strerror(nn_errno()));
  }
}

// NAN_METHOD(Symbol) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   int val;
//   const char *ret = nn_symbol(s, &val);

//   if (ret) {
//     Local<Object> obj = Nan::New<Object>();
//     Nan::Set(obj, Nan::New("value").ToLocalChecked(), Nan::New<Number>(val));
//     Nan::Set(obj, Nan::New("name").ToLocalChecked(),
//              Nan::New<String>(ret).ToLocalChecked());
//     info.GetReturnValue().Set(obj);
//   } else {
//     // symbol index out of range
//     // this behaviour seems inconsistent with SymbolInfo() above
//     // but we are faithfully following the libnanomsg API, warta and all
//     Nan::ThrowError(nn_strerror(nn_errno())); // EINVAL
//   }
// }

void Symbol(napi_env env, napi_func_cb_info info) {
  napi_value args[1];
  napi_get_cb_args(env, info, args, 1);

  int s = napi_get_value_int64(env, args[0]);
  int val;
  const char* ret = nn_symbol(s, &val);

  if (ret) {
    napi_value obj = napi_create_object();
    napi_set_property(env, obj, napi_property_name("value"),
                      napi_create_number(val));
    napi_set_property(env, obj, napi_property_name("name"),
                      napi_create_string(ret));
    napi_set_return_value(obj);
  } else {
    // symbol index out of range
    // this behaviour seems inconsistent with SymbolInfo() above
    // but we are faithfully following the libnanomsg API, warta and all
    napi_throw_error(env, nn_strerror(nn_errno()));  // EINVAL
  }
}

// NAN_METHOD(Term) { nn_term(); }

void Term(napi_env env, napi_func_cb_info info) {
  nn_term();
}

// // Pass in two sockets, or (socket, -1) or (-1, socket) for loopback
// NAN_METHOD(Device) {
//   int s1 = Nan::To<int>(info[0]).FromJust();
//   int s2 = Nan::To<int>(info[1]).FromJust();

//   // nn_device only returns when it encounters an error
//   nn_device(s1, s2);
//   Nan::ThrowError(nn_strerror(nn_errno()));
// }

// Pass in two sockets, or (socket, -1) or (-1, socket) for loopback
void Device(napi_env env, napi_func_cb_info info) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s1 = napi_get_value_int64(env, args[0]);
  int s2 = napi_get_value_int64(env, args[1]);

  // nn_device only returns when it encounters an error
  nn_device(s1, s2);
  napi_throw_error(env, nn_strerror(nn_errno()));
}

// NAN_METHOD(Errno) { info.GetReturnValue().Set(Nan::New<Number>(nn_errno())); }

void Errno(napi_env env, napi_func_cb_info info) {
  napi_set_return_value(env, napi_create_number(nn_errno()));
}

// NAN_METHOD(Err) {
//   info.GetReturnValue().Set(Nan::New(nn_strerror(nn_errno())).ToLocalChecked());
// }

void Err(napi_env env, napi_func_cb_info info) {
  napi_set_return_value(env, napi_create_string(nn_strerror(nn_errno())));
}

// typedef struct nanomsg_socket_s {
//   uv_poll_t poll_handle;
//   uv_os_sock_t sockfd;
//   Nan::Callback *callback;
// } nanomsg_socket_t;

typedef struct nanomsg_socket_s {
  uv_poll_t poll_handle;
  uv_os_sock_t sockfd;
  napi_callback *callback;
} nanomsg_socket_t;

// void NanomsgReadable(uv_poll_t *req, int status, int events) {
//   Nan::HandleScope scope;

//   nanomsg_socket_t *context;
//   context = reinterpret_cast<nanomsg_socket_t *>(req);

//   if (events & UV_READABLE) {
//     Local<Value> argv[] = { Nan::New<Number>(events) };
//     context->callback->Call(1, argv);
//   }
// }

void NanomsgReadable(uv_poll_t *req, int status, int events) {
  napi_env env = napi_get_current_env();

  nanomsg_socket_t *context;
  context = reinterpret_cast<nanomsg_socket_t *>(req);

  if (events & UV_READABLE) {
    napi_value argv[] = { napi_create_number(events) };
    napi_call_function(context->callback, 1, argv);
  }
}

// NAN_METHOD(PollSendSocket) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   Nan::Callback *callback = new Nan::Callback(info[1].As<Function>());

//   nanomsg_socket_t *context;
//   size_t siz = sizeof(uv_os_sock_t);

//   context = reinterpret_cast<nanomsg_socket_t *>(calloc(1, sizeof *context));
//   context->poll_handle.data = context;
//   context->callback = callback;
//   nn_getsockopt(s, NN_SOL_SOCKET, NN_SNDFD, &context->sockfd, &siz);

//   if (context->sockfd != 0) {
//     uv_poll_init_socket(uv_default_loop(), &context->poll_handle,
//                         context->sockfd);
//     uv_poll_start(&context->poll_handle, UV_READABLE, NanomsgReadable);
//     info.GetReturnValue().Set(WrapPointer(context, 8));
//   }
// }

NAPI_METHOD(PollSendSocket) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  napi_callback cb = napi_create_function(env, args[1]);

  nanomsg_socket_t *context;
  size_t siz = sizeof(uv_os_sock_t);

  context = reinterpret_cast<nanomsg_socket_t *>(calloc(1, sizeof *context));
  context->poll_handle.data = context;
  context->callback = callback;
  nn_getsockopt(s, NN_SOL_SOCKET, NN_SNDFD, &context->sockfd, &siz);

  if (context->sockfd != 0) {
    uv_poll_init_socket(uv_default_loop(), &context->poll_handle,
                        context->sockfd);
    uv_poll_start(&context->poll_handle, UV_READABLE, NanomsgReadable);
    napi_set_return_value(env, WrapPointer(context, 8));
  }
}

// NAN_METHOD(PollReceiveSocket) {
//   int s = Nan::To<int>(info[0]).FromJust();
//   Nan::Callback *callback = new Nan::Callback(info[1].As<Function>());

//   nanomsg_socket_t *context;
//   size_t siz = sizeof(uv_os_sock_t);

//   context = reinterpret_cast<nanomsg_socket_t *>(calloc(1, sizeof *context));
//   context->poll_handle.data = context;
//   context->callback = callback;
//   nn_getsockopt(s, NN_SOL_SOCKET, NN_RCVFD, &context->sockfd, &siz);

//   if (context->sockfd != 0) {
//     uv_poll_init_socket(uv_default_loop(), &context->poll_handle,
//                         context->sockfd);
//     uv_poll_start(&context->poll_handle, UV_READABLE, NanomsgReadable);
//     info.GetReturnValue().Set(WrapPointer(context, 8));
//   }
// }

NAPI_METHOD(PollReceiveSocket) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  napi_callback cb = napi_create_function(env, args[1]);

  nanomsg_socket_t *context;
  size_t siz = sizeof(uv_os_sock_t);

  context = reinterpret_cast<nanomsg_socket_t *>(calloc(1, sizeof *context));
  context->poll_handle.data = context;
  context->callback = callback;
  nn_getsockopt(s, NN_SOL_SOCKET, NN_RCVFD, &context->sockfd, &siz);

  if (context->sockfd != 0) {
    uv_poll_init_socket(uv_default_loop(), &context->poll_handle,
                        context->sockfd);
    uv_poll_start(&context->poll_handle, UV_READABLE, NanomsgReadable);
    napi_set_return_value(env, WrapPointer(context, 8));
  }
}

// NAN_METHOD(PollStop) {
//   nanomsg_socket_t *context = UnwrapPointer<nanomsg_socket_t *>(info[0]);
//   int r = uv_poll_stop(&context->poll_handle);
//   info.GetReturnValue().Set(Nan::New<Number>(r));
// }

NAPI_METHOD(PollStop) {
  napi_value args[1];
  napi_get_cb_args(env, info, args, 1);

  nanomsg_socket_t *context = UnwrapPointer<nanomsg_socket_t *>(args[0]);
  int r = uv_poll_stop(&context->poll_handle);
  napi_set_return_value(env, napi_create_number(r));
}

// class NanomsgDeviceWorker : public Nan::AsyncWorker {
// public:
//   NanomsgDeviceWorker(Nan::Callback *callback, int s1, int s2)
//       : Nan::AsyncWorker(callback), s1(s1), s2(s2) {}
//   ~NanomsgDeviceWorker() {}

//   // Executed inside the worker-thread.
//   // It is not safe to access V8, or V8 data structures
//   // here, so everything we need for input and output
//   // should go on `this`.
//   void Execute() {
//     // nn_errno() only returns on error
//     nn_device(s1, s2);
//     err = nn_errno();
//   }

//   // Executed when the async work is complete
//   // this function will be run inside the main event loop
//   // so it is safe to use V8 again
//   void HandleOKCallback() {
//     Nan::HandleScope scope;

//     Local<Value> argv[] = { Nan::New<Number>(err) };

//     callback->Call(1, argv);
//   };

// private:
//   int s1;
//   int s2;
//   int err;
// };

class NanomsgDeviceWorker : public AsyncWorker {
public:
  NanomsgDeviceWorker(napi_callback *callback, int s1, int s2)
      : AsyncWorker(callback), s1(s1), s2(s2) {}
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
  void HandleOKCallback() {
    napi_env env = napi_get_current_env();

    napi_value argv[] = { napi_create_number(err) };

    napi_call_function(env, callback, 1, argv);
  };

private:
  int s1;
  int s2;
  int err;
};

// // Asynchronous access to the `nn_device()` function
// NAN_METHOD(DeviceWorker) {
//   int s1 = Nan::To<int>(info[0]).FromJust();
//   int s2 = Nan::To<int>(info[1]).FromJust();
//   Nan::Callback *callback = new Nan::Callback(info[2].As<Function>());

//   Nan::AsyncQueueWorker(new NanomsgDeviceWorker(callback, s1, s2));
// }

// Asynchronous access to the `nn_device()` function
NAPI_METHOD(DeviceWorker) {
  napi_value args[3];
  napi_get_cb_args(env, info, args, 3);

  int s1 = napi_get_value_int64(env, args[0]);
  int s2 = napi_get_value_int64(env, args[1]);

  napi_callback *callback = napi_create_function(args[2]);

  AsyncQueueWorker(new NanomsgDeviceWorker(callback, s1, s2));
}

// #define EXPORT_METHOD(C, S)                                                    \
//   Nan::Set(C, Nan::New(#S).ToLocalChecked(),                                   \
//            Nan::GetFunction(Nan::New<FunctionTemplate>(S)).ToLocalChecked());

void InitAll(napi_env env, napi_value exports, napi_value module) {
  napi_env env = napi_get_current_env();
  napi_set_property(env, exports,
                        napi_property_name(env, "Socket"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Close"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Chan"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Bind"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Connect"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Shutdown"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Send"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Recv"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Errno"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "PollSendSocket"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "PollReceiveSocket"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "DeviceWorker"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "SymbolInfo"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Symbol"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Term"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Getopt"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Setopt"),
                        napi_create_function(env, Socket));
  napi_set_property(env, exports,
                        napi_property_name(env, "Err"),
                        napi_create_function(env, Socket));

  // TODO convert symbol loading into napi style
  // Export symbols.
  for (int i = 0;; ++i) {
    int value;
    const char *symbol_name = nn_symbol(i, &value);
    if (symbol_name == NULL) {
      break;
    }
    Nan::Set(target, Nan::New(symbol_name).ToLocalChecked(),
             Nan::New<Number>(value));
  }
}

NODE_MODULE_ABI(node_nanomsg, InitAll)
