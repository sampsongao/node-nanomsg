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
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int domain;
  status = napi_get_value_int32(env, args[0], &domain);
  CHECK_STATUS;
  int protocol;
  status = napi_get_value_int32(env, args[1], &protocol);
  CHECK_STATUS;
  napi_value ret;
  status = napi_create_number(env, nn_socket(domain, protocol), &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Close) {
  napi_status status;
  napi_value args[1];
  status = napi_get_cb_args(env, info, args, 1);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int rc = 0;

  do {
    rc = nn_close(s);
  } while (rc < 0 && errno == EINTR);

  napi_value ret;
  status = napi_create_number(env, rc, &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Setopt) {
  napi_status status;
  napi_value args[4];
  status = napi_get_cb_args(env, info, args, 4);

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int level;
  status = napi_get_value_int32(env, args[1], &level);
  CHECK_STATUS;
  int option;
  status = napi_get_value_int32(env, args[2], &option);
  CHECK_STATUS;
  int optval;
  status = napi_get_value_int32(env, args[3], &optval);
  CHECK_STATUS;
  
  int result = nn_setsockopt(s, level, option, &optval, sizeof(optval));
  napi_value ret;
  status = napi_create_number(env, result, &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Getopt) {
  napi_status status;
  napi_value args[3];
  status = napi_get_cb_args(env, info, args, 3);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int level;
  status = napi_get_value_int32(env, args[1], &level);
  CHECK_STATUS;
  int option;
  status = napi_get_value_int32(env, args[2], &option);
  CHECK_STATUS;
  int optval;
  size_t optsize = sizeof(optval);

  // check if the function succeeds
  if (nn_getsockopt(s, level, option, &optval, &optsize) == 0) {
    napi_value ret;
    status = napi_create_number(env, optval, &ret);
    CHECK_STATUS;
    status = napi_set_return_value(env, info, ret);
    CHECK_STATUS;
  }
}

NAPI_METHOD(Chan) {
  napi_status status;
  napi_value args[3];
  status = napi_get_cb_args(env, info, args, 3);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int level = NN_SUB;
  int option;
  status = napi_get_value_int32(env, args[1], &option);
  CHECK_STATUS;
  char str[1024];
  int remain;
  status = napi_get_string_from_value(env, args[2], str, 1024, &remain);
  CHECK_STATUS;
  int length;
  status = napi_get_string_utf8_length(env, args[2], &length);
  CHECK_STATUS;
 
  int result = nn_setsockopt(s, level, option, str, length);
  napi_value ret;
  status = napi_create_number(env, result, &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Bind) {
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  char addr[1024];
  int remain;
  status = napi_get_string_from_value(env, args[1], addr, 1024, &remain);
  CHECK_STATUS;
  napi_value ret;
  status = napi_create_number(env, nn_bind(s, addr), &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Connect) {
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  char addr[1024];
  int remain;
  status = napi_get_string_from_value(env, args[1], addr, 1024, &remain);
  CHECK_STATUS;

  napi_value ret;
  status = napi_create_number(env, nn_connect(s, addr), &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Shutdown) {
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int how;
  status = napi_get_value_int32(env, args[1], &how);
  CHECK_STATUS;

  napi_value ret;
  status = napi_create_number(env, nn_shutdown(s, how), &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Send) {
  napi_status status;
  napi_value args[3];
  status = napi_get_cb_args(env, info, args, 3);

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int flags;
  status = napi_get_value_int32(env, args[2], &flags);
  CHECK_STATUS;

  bool has_instance;
  status = napi_buffer_has_instance(env, args[1], &has_instance);
  CHECK_STATUS;
  if (has_instance) {
    char* buf;
    status = napi_buffer_data(env, args[1], &buf);
    CHECK_STATUS;
    size_t size;
    status = napi_buffer_length(env, args[1], &size);
    CHECK_STATUS;
    int result = nn_send(s, buf, size, flags);
    napi_value ret;
    status = napi_create_number(env, result, &ret);
    CHECK_STATUS;
    status = napi_set_return_value(env, info, ret);
    CHECK_STATUS;
  } else {
    char str[1024];
    int remain;
    status = napi_get_string_from_value(env, args[1], str, 1024, &remain);
    CHECK_STATUS;
    int length = strlen(str);
    int result = nn_send(s, str, length, flags);
    napi_value ret;
    status = napi_create_number(env, result, &ret);
    CHECK_STATUS;
    status = napi_set_return_value(env, info, ret);
    CHECK_STATUS;
  }
}

NAPI_METHOD(Recv) {
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int flags;
  status = napi_get_value_int32(env, args[1], &flags);
  CHECK_STATUS;

  char* buf = NULL;
  int len = nn_recv(s, &buf, NN_MSG, flags);

  if (len > -1) {
    napi_value h;
    status = napi_buffer_copy(env, buf, len, &h);
    CHECK_STATUS;
    status = napi_set_return_value(env, info, h);
    CHECK_STATUS;
  } else {
    napi_value ret;
    status = napi_create_number(env, len, &ret);
    CHECK_STATUS;
    status = napi_set_return_value(env, info, ret);
    CHECK_STATUS;
  }
}

NAPI_METHOD(SymbolInfo) {
  napi_status status;
  napi_value args[1];
  status = napi_get_cb_args(env, info, args, 1);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;

  struct nn_symbol_properties prop;
  int ret = nn_symbol_info(s, &prop, sizeof(prop));

  if (ret > 0) {
    napi_value obj;
    status = napi_create_object(env, &obj);
    CHECK_STATUS;
    napi_propertyname pro;
    napi_value val;
    status = napi_property_name(env, "value", &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.value, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_property_name(env, "ns", &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.ns, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_property_name(env, "type", &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.type, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_property_name(env, "unit", &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.unit, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_property_name(env, "name", &pro);
    CHECK_STATUS;
    status = napi_create_string(env, prop.name, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS

    status = napi_set_return_value(env, info, obj);
    CHECK_STATUS
  } else if (ret != 0) {
    status = napi_throw_error(env, (char*) nn_strerror(nn_errno()));
    CHECK_STATUS
  }
}

NAPI_METHOD(Symbol) {
  napi_status status;
  napi_value args[1];
  status = napi_get_cb_args(env, info, args, 1);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int value;
  const char* ret = nn_symbol(s, &value);

  if (ret) {
    napi_value obj;
    status = napi_create_object(env, &obj);
    CHECK_STATUS;
    napi_propertyname pro;
    napi_value val;
    status = napi_property_name(env, "value", &pro);
    CHECK_STATUS;
    status = napi_create_number(env, value, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS;
    status = napi_property_name(env, "name", &pro);
    CHECK_STATUS;
    status = napi_create_string(env, ret, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS;
    status = napi_set_return_value(env, info, obj);
    CHECK_STATUS;
  } else {
    // symbol index out of range
    // this behaviour seems inconsistent with SymbolInfo() above
    // but we are faithfully following the libnanomsg API, warta and all
    status = napi_throw_error(env, (char*) nn_strerror(nn_errno()));  // EINVAL
    CHECK_STATUS;
  }
}

NAPI_METHOD(Term) {
  nn_term();
}

// Pass in two sockets, or (socket, -1) or (-1, socket) for loopback
NAPI_METHOD(Device) {
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int s1;
  status = napi_get_value_int32(env, args[0], &s1);
  CHECK_STATUS;
  int s2;
  status = napi_get_value_int32(env, args[1], &s2);
  CHECK_STATUS;

  // nn_device only returns when it encounters an error
  nn_device(s1, s2);
  status = napi_throw_error(env, (char*) nn_strerror(nn_errno()));
  CHECK_STATUS;
}

NAPI_METHOD(Errno) {
  napi_status status;
  napi_value ret;
  status = napi_create_number(env, nn_errno(), &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Err) {
  napi_status status;
  napi_value str;
  status = napi_create_string(env, (char*) nn_strerror(nn_errno()), &str);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, str);
  CHECK_STATUS;
}

typedef struct nanomsg_socket_s {
  uv_poll_t poll_handle;
  uv_os_sock_t sockfd;
  Napi::Callback *callback;
} nanomsg_socket_t;

void NanomsgReadable(uv_poll_t *req, int status, int events) {
  Napi::HandleScope s;
  napi_env env;
  status = napi_get_current_env(&env);
  CHECK_STATUS;

  nanomsg_socket_t *context;
  context = reinterpret_cast<nanomsg_socket_t *>(req);

  if (events & UV_READABLE) {
    napi_value argv[1];
    status = napi_create_number(env, events, argv);
    CHECK_STATUS;
    napi_value scope;
    status = napi_get_global(env, &scope);
    CHECK_STATUS;
    context->callback->Call(scope, 1, argv);
  }
}

NAPI_METHOD(PollSendSocket) {
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
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
    status = napi_set_return_value(env, info, WrapPointer(context, 8));
    CHECK_STATUS;
  }
}

NAPI_METHOD(PollReceiveSocket) {
  napi_status status;
  napi_value args[2];
  status = napi_get_cb_args(env, info, args, 2);
  CHECK_STATUS;

  int s;
  status = napi_get_value_int32(env, args[0], &s);
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
    status = napi_set_return_value(env, info, WrapPointer(context, 8));
    CHECK_STATUS;
  }
}

NAPI_METHOD(PollStop) {
  napi_status status;
  napi_value args[1];
  status = napi_get_cb_args(env, info, args, 1);
  CHECK_STATUS;

  nanomsg_socket_t *context = UnwrapPointer<nanomsg_socket_t *>(args[0]);
  int r = uv_poll_stop(&context->poll_handle);
  napi_value ret;
  status = napi_create_number(env, r, &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
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
    Napi::HandleScope s;
    napi_status status;
    napi_env env;
    status = napi_get_current_env(&env);
    CHECK_STATUS;

    napi_value argv[1];
    status = napi_create_number(env, err, argv);
    CHECK_STATUS;

    napi_value scope;
    status = napi_get_global(env, &scope);
    CHECK_STATUS;
    callback->Call(scope, 1, argv);
  };

private:
  int s1;
  int s2;
  int err;
};

// Asynchronous access to the `nn_device()` function
NAPI_METHOD(DeviceWorker) {
  napi_status status;
  napi_value args[3];
  status = napi_get_cb_args(env, info, args, 3);
  CHECK_STATUS;

  int s1;
  status = napi_get_value_int32(env, args[0], &s1);
  CHECK_STATUS;
  int s2;
  status = napi_get_value_int32(env, args[1], &s2);
  CHECK_STATUS;

  Napi::Callback* callback = new Napi::Callback(args[2]);

  Napi::AsyncQueueWorker(new NanomsgDeviceWorker(callback, s1, s2));
}

#define EXPORT_METHOD(C, S)                                                    \
  status = napi_property_name(env, #S, &pro);                                           \
  CHECK_STATUS;                                                                \
  status = napi_create_function(env, S, nullptr, &val);                                 \
  CHECK_STATUS;                                                                \
  status = napi_set_property(env, C, pro, val);                                         \
  CHECK_STATUS;

NAPI_MODULE_INIT(InitAll) {
  Napi::HandleScope scope;
  napi_status status;
  napi_propertyname pro;
  napi_value val;

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
    status = napi_property_name(env, symbol_name, &pro);
    CHECK_STATUS;
    status = napi_create_number(env, value, &val);
    CHECK_STATUS;
    status = napi_set_property(env, exports, pro, val);
    CHECK_STATUS;
  }
}

NODE_MODULE_ABI(node_nanomsg, InitAll)
