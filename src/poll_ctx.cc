#include "nn.h"
#include "poll_ctx.h"

static void NapiomsgReadable(uv_poll_t* req, int /* status */, int events) {
  const PollCtx* const context = static_cast<PollCtx*>(req->data);
  if (events & UV_READABLE) {
    context->invoke_callback(events);
  }
}

void PollCtx::begin_poll (const int s, const bool is_sender) {
  size_t siz = sizeof(uv_os_sock_t);
  nn_getsockopt(s, NN_SOL_SOCKET, is_sender ? NN_SNDFD : NN_RCVFD, &sockfd,
      &siz);
  if (sockfd != 0) {
    uv_poll_init_socket(uv_default_loop(), &poll_handle, sockfd);
    uv_poll_start(&poll_handle, UV_READABLE, NapiomsgReadable);
  }
}

PollCtx::PollCtx (const int s, const bool is_sender,
  const napi_value cb): callback(cb) {
  // TODO: maybe container_of can be used instead?
  // that would save us this assignment, and ugly static_cast hacks.
  poll_handle.data = this;
  begin_poll(s, is_sender);
}

void PollCtx::invoke_callback (const int events) const {
  Napi::HandleScope scope;
  napi_status status;
  napi_env env;
  status = napi_get_current_env(&env);
  CHECK_STATUS;

  napi_value argv[1];
  status = napi_create_number(env, events, argv);
  CHECK_STATUS;
  callback.Call(1, argv);
}

// Napi will invoke this once it's done with the Buffer, in case we wanted to
// free ptr.  In this case, ptr is a PollCtx that we're not done with and don't
// want to free yet (not until PollStop is invoked), so we do nothing.
static void wrap_pointer_cb(void * /* data */) {}

napi_value PollCtx::WrapPointer (void* ptr, size_t length) {
  napi_status status;
  napi_env env;
  status = napi_get_current_env(&env);
  CHECK_STATUS;
  napi_value buf;
  status = napi_create_external_buffer(env, length, static_cast<char *>(ptr), wrap_pointer_cb, &buf);
  CHECK_STATUS;
  return buf;
}

PollCtx* PollCtx::UnwrapPointer (napi_value buffer) {
  napi_status status;
  napi_env env;
  status = napi_get_current_env(&env);
  CHECK_STATUS;
  bool has_instance;
  status = napi_is_buffer(env, buffer, &has_instance);
  CHECK_STATUS;
  if (has_instance) {
    char* data;
    size_t size;
    status = napi_get_buffer_info(env, buffer, &data, &size);
    CHECK_STATUS;
    return reinterpret_cast<PollCtx*>(data);
  } else {
    return NULL;
  }
}
