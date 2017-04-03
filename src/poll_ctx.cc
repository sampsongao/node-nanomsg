#include "nn.h"
#include "poll_ctx.h"

static void NanomsgReadable(uv_poll_t* req, int /* status */, int events) {
  const PollCtx* const context = static_cast<PollCtx*>(req->data);
  if (events & UV_READABLE) {
    context->invoke_callback(context->env, events);
  }
}

void PollCtx::begin_poll (napi_env env, const int s, const bool is_sender) {
  size_t siz = sizeof(uv_os_sock_t);
  nn_getsockopt(s, NN_SOL_SOCKET, is_sender ? NN_SNDFD : NN_RCVFD, &sockfd,
      &siz);
  if (sockfd != 0) {
    uv_poll_init_socket(uv_default_loop(), &poll_handle, sockfd);
    uv_poll_start(&poll_handle, UV_READABLE, NanomsgReadable);
  }
}

PollCtx::PollCtx (napi_env _env, const int s,
                  const bool is_sender, napi_value cb) {
  // TODO: maybe container_of can be used instead?
  // that would save us this assignment, and ugly static_cast hacks.
  env = _env;
  napi_status status;
  status = napi_create_reference(env, cb, 1, &handle);
  CHECK_STATUS;
  poll_handle.data = this;
  begin_poll(env, s, is_sender);
}

void PollCtx::invoke_callback (napi_env env, const int events) const {
  napi_status status;
  napi_handle_scope scope;
  status = napi_open_handle_scope(env, &scope);
  CHECK_STATUS;

  napi_value argv[1];
  status = napi_create_number(env, events, argv);
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

// Napi will invoke this once it's done with the Buffer, in case we wanted to
// free ptr.  In this case, ptr is a PollCtx that we're not done with and don't
// want to free yet (not until PollStop is invoked), so we do nothing.
static void wrap_pointer_cb(napi_env, void * /* data */, void* /* hint */) {}

napi_value PollCtx::WrapPointer (napi_env env, void* ptr, size_t length) {
  napi_status status;
  napi_value buf;
  status = napi_create_external_buffer(env, length, ptr, wrap_pointer_cb, nullptr, &buf);
  CHECK_STATUS;
  return buf;
}

PollCtx* PollCtx::UnwrapPointer(napi_env env, napi_value buffer) {
  napi_status status;
  bool has_instance;
  status = napi_is_buffer(env, buffer, &has_instance);
  CHECK_STATUS;
  if (has_instance) {
    void* data;
    size_t size;
    status = napi_get_buffer_info(env, buffer, &data, &size);
    CHECK_STATUS;
    return reinterpret_cast<PollCtx*>(data);
  } else {
    return NULL;
  }
}
