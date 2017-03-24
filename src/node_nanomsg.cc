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

#include "macros.h"
#include <string.h>

NAPI_METHOD(Socket) {
  BEGIN(2);

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
  BEGIN(1);
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
  BEGIN(4);
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
  BEGIN(4);
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
  BEGIN(3);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int level = NN_SUB;
  int option;
  status = napi_get_value_int32(env, args[1], &option);
  CHECK_STATUS;
  char str[1024];
  size_t copied;
  status = napi_get_value_string_utf8(env, args[2], str, 1024, &copied);
  CHECK_STATUS;
  size_t length;
  status = napi_get_value_string_length(env, args[2], &length);
  CHECK_STATUS;

  int result = nn_setsockopt(s, level, option, str, length);
  napi_value ret;
  status = napi_create_number(env, result, &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Bind) {
  BEGIN(2);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  char addr[1024];
  size_t copied;
  status = napi_get_value_string_utf8(env, args[1], addr, 1024, &copied);
  CHECK_STATUS;
  napi_value ret;
  status = napi_create_number(env, nn_bind(s, addr), &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Connect) {
  BEGIN(2);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  char addr[1024];
  size_t copied;
  status = napi_get_value_string_utf8(env, args[1], addr, 1024, &copied);
  CHECK_STATUS;

  napi_value ret;
  status = napi_create_number(env, nn_connect(s, addr), &ret);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

NAPI_METHOD(Shutdown) {
  BEGIN(2);
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
  BEGIN(3);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int flags;
  status = napi_get_value_int32(env, args[2], &flags);
  CHECK_STATUS;

  bool has_instance;
  status = napi_is_buffer(env, args[1], &has_instance);
  CHECK_STATUS;
  if (has_instance) {
    char* buf;
    size_t size;
    status = napi_get_buffer_info(env, args[1],
                                  reinterpret_cast<void**>(&buf), &size);
    CHECK_STATUS;
    int result = nn_send(s, buf, size, flags);
    napi_value ret;
    status = napi_create_number(env, result, &ret);
    CHECK_STATUS;
    status = napi_set_return_value(env, info, ret);
    CHECK_STATUS;
  } else {
    napi_value string;
    status = napi_coerce_to_string(env, args[1], &string);
    CHECK_STATUS;
    char str[1024];
    size_t copied;
    status = napi_get_value_string_utf8(env, string, str, 1024, &copied);
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

static void fcb(char *data, void *) {
  nn_freemsg(data);
}

NAPI_METHOD(Recv) {
  BEGIN(2);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int flags;
  status = napi_get_value_int32(env, args[1], &flags);
  CHECK_STATUS;

  char* buf = NULL;
  const int len = nn_recv(s, &buf, NN_MSG, flags);

  if (len > -1) {
    napi_value h;
    status = napi_create_buffer_copy(env, static_cast<const size_t>(len),
                                     buf, NULL, &h);
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
  BEGIN(1);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;

  struct nn_symbol_properties prop;
  int ret = nn_symbol_info(s, &prop, sizeof(prop));
  if (ret > 0) {
    napi_value obj;
    status = napi_create_object(env, &obj);
    CHECK_STATUS;
    napi_value pro;
    napi_value val;
    status = napi_create_string_utf8(env, "value", -1, &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.value, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_create_string_utf8(env, "ns", -1, &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.ns, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_create_string_utf8(env, "type", -1, &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.type, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_create_string_utf8(env, "unit", -1, &pro);
    CHECK_STATUS;
    status = napi_create_number(env, prop.unit, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS
    status = napi_create_string_utf8(env, "name", -1, &pro);
    CHECK_STATUS;
    status = napi_create_string_utf8(env, prop.name, -1, &val);
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
  BEGIN(1);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  int value;
  const char* ret = nn_symbol(s, &value);

  if (ret) {
    napi_value obj;
    status = napi_create_object(env, &obj);
    CHECK_STATUS;
    napi_value pro;
    napi_value val;
    status = napi_create_string_utf8(env, "value", -1, &pro);
    CHECK_STATUS;
    status = napi_create_number(env, value, &val);
    CHECK_STATUS;
    status = napi_set_property(env, obj, pro, val);
    CHECK_STATUS;
    status = napi_create_string_utf8(env, "name", -1, &pro);
    CHECK_STATUS;
    status = napi_create_string_utf8(env, ret, -1, &val);
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

NAPI_METHOD(Term) { nn_term(); }

// Pass in two sockets, or (socket, -1) or (-1, socket) for loopback
NAPI_METHOD(Device) {
  BEGIN(2);
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
  status = napi_create_string_utf8(env, (char*) nn_strerror(nn_errno()), -1, &str);
  CHECK_STATUS;
  status = napi_set_return_value(env, info, str);
  CHECK_STATUS;
}

NAPI_METHOD(PollSocket) {
  BEGIN(3);
  int s;
  status = napi_get_value_int32(env, args[0], &s);
  CHECK_STATUS;
  bool is_sender;
  status = napi_get_value_bool(env, args[1], &is_sender);
  CHECK_STATUS;
  PollCtx *context = new PollCtx(env, s, is_sender, args[2]);
  napi_value ret = PollCtx::WrapPointer(env, context, sizeof context);
  status = napi_set_return_value(env, info, ret);
  CHECK_STATUS;
}

static void close_cb(uv_handle_t *handle) {
  const PollCtx* const context = static_cast<PollCtx*>(handle->data);
  delete context;
}

NAPI_METHOD(PollStop) {
  BEGIN(1);
  PollCtx* const context = PollCtx::UnwrapPointer(env, args[0]);
  if (context != NULL) {
    uv_close(reinterpret_cast<uv_handle_t*>(&context->poll_handle), close_cb);
  }
  // TODO: the else case should never happen.  Maybe add an assert or
  // something.
}

class NanomsgDeviceWorker {
public:
  NanomsgDeviceWorker(napi_env _env, napi_value _fn, int _s1, int _s2) {
    env = _env;
    napi_create_reference(env, _fn, 1, &handle);
    s1 = _s1;
    s2 = _s2;
    err = 0;
    request = napi_create_async_work();
  }

  ~NanomsgDeviceWorker() {
    napi_status status;
    status = napi_delete_reference(env, handle);
    CHECK_STATUS;
    napi_delete_async_work(request);
  }

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute() {
    // nn_errno() only returns on error
    nn_device(s1, s2);
    err = nn_errno();
  }

  void WorkComplete() {
    napi_status status;
    napi_handle_scope scope;
    status = napi_open_handle_scope(env, &scope);
    CHECK_STATUS;

    if (err != 0)
      HandleErrorCallback();
    else
      HandleOKCallback();

    status = napi_close_handle_scope(env, scope);
    CHECK_STATUS;
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback() {
    napi_status status;
    napi_handle_scope scope;
    status = napi_open_handle_scope(env, &scope);
    CHECK_STATUS;;

    napi_value argv[1];
    status = napi_create_number(env, err, argv);
    CHECK_STATUS;

    napi_value global;
    status = napi_get_global(env, &global);
    CHECK_STATUS;
    napi_value fn;
    status = napi_get_reference_value(env, handle, &fn);
    napi_value result;
    status = napi_make_callback(env, global, fn, 1, argv, &result);
    CHECK_STATUS;
    status = napi_close_handle_scope(env, scope);
    CHECK_STATUS;
  };

  void HandleErrorCallback() {
    napi_status status;
    napi_handle_scope scope;
    status = napi_open_handle_scope(env, &scope);
    CHECK_STATUS;;

    napi_value n;
    status = napi_create_number(env, err, &n);
    CHECK_STATUS;
    napi_value s;
    status = napi_coerce_to_string(env, n, &s);
    CHECK_STATUS;

    napi_value argv[1];
    status = napi_create_error(env, s, argv);
    CHECK_STATUS;

    napi_value global;
    status = napi_get_global(env, &global);
    CHECK_STATUS;
    napi_value fn;
    status = napi_get_reference_value(env, handle, &fn);
    napi_value result;
    status = napi_make_callback(env, global, fn, 1, argv, &result);
    CHECK_STATUS;

    status = napi_close_handle_scope(env, scope);
    CHECK_STATUS;
  }

  napi_work request;

  static void CallExecute(void* this_pointer){
    NanomsgDeviceWorker* self = static_cast<NanomsgDeviceWorker*>(this_pointer);
    self->Execute();
  }

  static void CallWorkComplete(void* this_pointer) {
    NanomsgDeviceWorker* self = static_cast<NanomsgDeviceWorker*>(this_pointer);
    self->WorkComplete();
  }


  static inline void AsyncQueueWorker(NanomsgDeviceWorker* worker) {
    napi_work req = worker->request;
    napi_async_set_data(req, static_cast<void*>(worker));
    napi_async_set_execute(req, &NanomsgDeviceWorker::CallExecute);
    napi_async_set_complete(req, &NanomsgDeviceWorker::CallWorkComplete);
    napi_async_queue_worker(req);
  }

private:
  napi_env env;
  napi_ref handle;
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

  NanomsgDeviceWorker::AsyncQueueWorker(new NanomsgDeviceWorker(env, args[2], s1, s2));
}

#define EXPORT_METHOD(C, S)                                            \
  status = napi_create_string_utf8(env, #S, -1, &pro);                 \
  CHECK_STATUS;                                                        \
  status = napi_create_function(env, #C, S, nullptr, &val);             \
  CHECK_STATUS;                                                        \
  status = napi_set_property(env, C, pro, val);                        \
  CHECK_STATUS;


NAPI_MODULE_INIT(InitAll) {
  napi_status status;
  napi_handle_scope scope;
  status = napi_open_handle_scope(env, &scope);
  CHECK_STATUS;;
  napi_value pro;
  napi_value val;


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
    status = napi_create_string_utf8(env, symbol_name, -1, &pro);
    CHECK_STATUS;
    status = napi_create_number(env, value, &val);
    CHECK_STATUS;
    status = napi_set_property(env, exports, pro, val);
    CHECK_STATUS;
  }

  status = napi_close_handle_scope(env, scope);
  CHECK_STATUS;
}

NAPI_MODULE(node_nanomsg, InitAll)
