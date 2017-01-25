#include <stdio.h>
#include <stdlib.h>
#include "node_pointer.h"

#include <nn.h>
#include <uv.h>

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

#include <node_api_helpers.h>

NAPI_METHOD(Socket) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int domain = napi_get_value_int64(env, args[0]);
  int protocol = napi_get_value_int64(env, args[1]);
  napi_value ret = napi_create_number(env, nn_socket(domain, protocol));
  napi_set_return_value(env, info, ret);
}

NAPI_METHOD(Close) {
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

NAPI_METHOD(Setopt) {
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

NAPI_METHOD(Getopt) {
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
    napi_set_return_value(env, info, ret);
  }
}

NAPI_METHOD(Chan) {
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

NAPI_METHOD(Bind) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  char addr[1024];
  int remain = napi_get_string_from_value(env, args[1], addr, 1024);
  napi_value ret = napi_create_number(env, nn_bind(s, addr));
  napi_set_return_value(env, info, ret);
}

NAPI_METHOD(Connect) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  char addr[1024];
  napi_get_string_from_value(env, args[1], addr, 1024);

  napi_value ret = napi_create_number(env, nn_connect(s, addr));
  napi_set_return_value(env, info, ret);
}

NAPI_METHOD(Shutdown) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  int how = napi_get_value_int64(env, args[1]);

  napi_value ret = napi_create_number(env, nn_shutdown(s, how));
  napi_set_return_value(env, info, ret);
}

NAPI_METHOD(Send) {
  napi_value args[3];
  napi_get_cb_args(env, info, args, 3);

  int s = napi_get_value_int64(env, args[0]);
  int flags = napi_get_value_int64(env, args[2]);

  if (napi_buffer_has_instance(env, args[1])) {
    int result = nn_send(s, napi_buffer_data(env, args[1]),
                         napi_buffer_length(env, args[1]), flags);
    napi_value ret = napi_create_number(env, result);
    napi_set_return_value(env, info, ret);
  } else {
    char str[1024];
    int remain = napi_get_string_from_value(env, args[1], str, 1024);
    int length = strlen(str);
    int result = nn_send(s, str, length, flags);
    napi_value ret = napi_create_number(env, result);
    napi_set_return_value(env, info, ret);
  }
}

NAPI_METHOD(Recv) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  int flags = napi_get_value_int64(env, args[1]);

  char* buf = NULL;
  int len = nn_recv(s, &buf, NN_MSG, flags);

  if (len > -1) {
    napi_value h = napi_buffer_copy(env, buf, len);
    napi_set_return_value(env, info, h);
  } else {
    napi_set_return_value(env, info, napi_create_number(env, len));
  }
}

NAPI_METHOD(SymbolInfo) {
  napi_value args[1];
  napi_get_cb_args(env, info, args, 1);

  int s = napi_get_value_int64(env, args[0]);

  struct nn_symbol_properties prop;
  int ret = nn_symbol_info(s, &prop, sizeof(prop));

  if (ret > 0) {
    napi_value obj = napi_create_object(env);
    napi_set_property(env, obj, napi_property_name(env, "value"), 
                      napi_create_number(env, prop.value));
    napi_set_property(env, obj, napi_property_name(env, "ns"), 
                      napi_create_number(env, prop.ns));
    napi_set_property(env, obj, napi_property_name(env, "type"), 
                      napi_create_number(env, prop.type));
    napi_set_property(env, obj, napi_property_name(env, "unit"), 
                      napi_create_number(env, prop.unit));
    napi_set_property(env, obj, napi_property_name(env, "name"), 
                      napi_create_string(env, prop.name));

    napi_set_return_value(env, info, obj);
  } else if (ret != 0) {
    napi_throw_error(env, (char*) nn_strerror(nn_errno()));
  }
}

NAPI_METHOD(Symbol) {
  napi_value args[1];
  napi_get_cb_args(env, info, args, 1);

  int s = napi_get_value_int64(env, args[0]);
  int val;
  const char* ret = nn_symbol(s, &val);

  if (ret) {
    napi_value obj = napi_create_object(env);
    napi_set_property(env, obj, napi_property_name(env, "value"),
                      napi_create_number(env, val));
    napi_set_property(env, obj, napi_property_name(env, "name"),
                      napi_create_string(env, ret));
    napi_set_return_value(env, info, obj);
  } else {
    // symbol index out of range
    // this behaviour seems inconsistent with SymbolInfo() above
    // but we are faithfully following the libnanomsg API, warta and all
    napi_throw_error(env, (char*) nn_strerror(nn_errno()));  // EINVAL
  }
}

NAPI_METHOD(Term) {
  nn_term();
}

// Pass in two sockets, or (socket, -1) or (-1, socket) for loopback
NAPI_METHOD(Device) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s1 = napi_get_value_int64(env, args[0]);
  int s2 = napi_get_value_int64(env, args[1]);

  // nn_device only returns when it encounters an error
  nn_device(s1, s2);
  napi_throw_error(env, (char*) nn_strerror(nn_errno()));
}

NAPI_METHOD(Errno) {
  napi_set_return_value(env, info, napi_create_number(env, nn_errno()));
}

NAPI_METHOD(Err) {
  napi_set_return_value(env, info,
                        napi_create_string(env,
                        (char*) nn_strerror(nn_errno())));
}

typedef struct nanomsg_socket_s {
  uv_poll_t poll_handle;
  uv_os_sock_t sockfd;
  Napi::Callback *callback;
} nanomsg_socket_t;

void NanomsgReadable(uv_poll_t *req, int status, int events) {
  Napi::HandleScope scope;
  napi_env env = napi_get_current_env();

  nanomsg_socket_t *context;
  context = reinterpret_cast<nanomsg_socket_t *>(req);

  if (events & UV_READABLE) {
    napi_value argv[] = { napi_create_number(env, events) };
    context->callback->Call(napi_get_global_scope(env), 1, argv);
  }
}

NAPI_METHOD(PollSendSocket) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  Napi::Callback* callback = new Napi::Callback(args[1]);

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
    napi_set_return_value(env, info, WrapPointer(context, 8));
  }
}

NAPI_METHOD(PollReceiveSocket) {
  napi_value args[2];
  napi_get_cb_args(env, info, args, 2);

  int s = napi_get_value_int64(env, args[0]);
  Napi::Callback* callback = new Napi::Callback(args[1]);

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
    napi_set_return_value(env, info, WrapPointer(context, 8));
  }
}

NAPI_METHOD(PollStop) {
  napi_value args[1];
  napi_get_cb_args(env, info, args, 1);

  nanomsg_socket_t *context = UnwrapPointer<nanomsg_socket_t *>(args[0]);
  int r = uv_poll_stop(&context->poll_handle);
  napi_set_return_value(env, info, napi_create_number(env, r));
}

class NanomsgDeviceWorker : public Napi::AsyncWorker {
public:
  NanomsgDeviceWorker(Napi::Callback *callback, int s1, int s2)
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
  void HandleOKCallback() {
    napi_env env = napi_get_current_env();

    napi_value argv[] = { napi_create_number(env, err) };

    callback->Call(napi_get_global_scope(env), 1, argv);
  };

private:
  int s1;
  int s2;
  int err;
};

// Asynchronous access to the `nn_device()` function
NAPI_METHOD(DeviceWorker) {
  napi_value args[3];
  napi_get_cb_args(env, info, args, 3);

  int s1 = napi_get_value_int64(env, args[0]);
  int s2 = napi_get_value_int64(env, args[1]);

  Napi::Callback* callback = new Napi::Callback(args[2]);

  Napi::AsyncQueueWorker(new NanomsgDeviceWorker(callback, s1, s2));
}

#define EXPORT_METHOD(C, S)                                                    \
   napi_set_property(env, C,                                                   \
                     napi_property_name(env, #S),                              \
                     napi_create_function(env, S, nullptr));

NAPI_MODULE_INIT(InitAll) {

  EXPORT_METHOD(exports, Socket);
  EXPORT_METHOD(exports, Close);
  EXPORT_METHOD(exports, Chan);
  EXPORT_METHOD(exports, Bind);
  EXPORT_METHOD(exports, Connect);
  EXPORT_METHOD(exports, Shutdown);
  EXPORT_METHOD(exports, Send);
  EXPORT_METHOD(exports, Recv);
  EXPORT_METHOD(exports, Errno);
  EXPORT_METHOD(exports, PollSendSocket);
  EXPORT_METHOD(exports, PollReceiveSocket);
  EXPORT_METHOD(exports, PollStop);
  EXPORT_METHOD(exports, DeviceWorker);
  EXPORT_METHOD(exports, SymbolInfo);
  EXPORT_METHOD(exports, Symbol);
  EXPORT_METHOD(exports, Term);

  EXPORT_METHOD(exports, Getopt);
  EXPORT_METHOD(exports, Setopt);
  EXPORT_METHOD(exports, Err);

  // TODO convert symbol loading into napi style
  // Export symbols.
  for (int i = 0;; ++i) {
    int value;
    const char *symbol_name = nn_symbol(i, &value);
    if (symbol_name == NULL) {
      break;
    }
    napi_set_property(env, exports,
                      napi_property_name(env, symbol_name),
                      napi_create_number(env, value));
  }
}

NODE_MODULE_ABI(node_nanomsg, InitAll)
