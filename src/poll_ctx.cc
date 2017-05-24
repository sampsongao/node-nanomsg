#include "nn.h"
#include "poll_ctx.h"

using Napi::Function;
using Napi::Number;
using Napi::Value;

static void NanomsgReadable(uv_poll_t* req, int /* status */, int events) {
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
    uv_poll_start(&poll_handle, UV_READABLE, NanomsgReadable);
  }
}

PollCtx::PollCtx (const int s, const bool is_sender,
    const Napi::Function cb) {
  // TODO: maybe container_of can be used instead?
  // that would save us this assignment, and ugly static_cast hacks.
  poll_handle.data = this;
  begin_poll(s, is_sender);
  callback.Reset(cb, 1);
}

void PollCtx::invoke_callback (const int events) const {
  Napi::HandleScope scope(callback.Env());
  callback.MakeCallback(callback.Env().Global(), { Napi::Number::New(callback.Env(), events) });
}

// Nan will invoke this once it's done with the Buffer, in case we wanted to
// free ptr.  In this case, ptr is a PollCtx that we're not done with and don't
// want to free yet (not until PollStop is invoked), so we do nothing.
static void wrap_pointer_cb(napi_env env, void * /* data */, void * /* hint */) {}

Napi::Value PollCtx::WrapPointer (Napi::Env env, void* ptr, size_t length) {
   return Napi::Buffer<char>::New(env, static_cast<char *>(ptr), length);
}

PollCtx* PollCtx::UnwrapPointer (Napi::Value buffer) {
  return reinterpret_cast<PollCtx*>(buffer.IsBuffer() ?
    buffer.As<Napi::Buffer<char>>().Data() : NULL);
}
