#pragma once

#include "macros.h"
#include <node_api_helpers.h>
#include <uv.h>

class PollCtx {
  private:
    const Napi::Callback callback;
    uv_os_sock_t sockfd; // for libnanomsg
    void begin_poll (const int s, const bool is_sender);
  public:
    uv_poll_t poll_handle; // for libuv
    PollCtx (int s, bool is_sender, napi_value cb);
    void invoke_callback (const int events) const;
    static napi_value WrapPointer (void* ptr, size_t length);
    static PollCtx* UnwrapPointer (napi_value buffer);
};
