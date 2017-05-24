#pragma once

#include <napi.h>
#include <uv.h>

class PollCtx {
  private:
    Napi::FunctionReference callback;
    uv_os_sock_t sockfd; // for libnanomsg
    void begin_poll (const int s, const bool is_sender);
  public:
    uv_poll_t poll_handle; // for libuv
    PollCtx (const int s, const bool is_sender,
        const Napi::Function cb);
    void invoke_callback (const int events) const;
    static Napi::Value WrapPointer (Napi::Env env, void* ptr, size_t length);
    static PollCtx* UnwrapPointer (Napi::Value buffer);
};
