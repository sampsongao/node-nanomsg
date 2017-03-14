#pragma once

#include "macros.h"
#include <node_api.h>
#include <node_api_async.h>
#include <uv.h>

class PollCtx {
  private:
    napi_ref handle;
    uv_os_sock_t sockfd; // for libnanomsg
    void begin_poll (napi_env env, const int s, const bool is_sender);
  public:
    napi_env env;
    uv_poll_t poll_handle; // for libuv
    PollCtx (napi_env env, int s, bool is_sender, napi_value cb);
    void invoke_callback (napi_env env, const int events) const;
    static napi_value WrapPointer (napi_env env, void* ptr, size_t length);
    static PollCtx* UnwrapPointer (napi_env env, napi_value buffer);
};
