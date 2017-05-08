#ifndef SRC_MACRO_H
#define SRC_MACRO_H
#include <assert.h>

#define NAPI_METHOD(name)                                                      \
  void name(napi_env env, napi_callback_info info)
#define NAPI_GETTER(name) NAPI_METHOD(name)
#define NAPI_SETTER(name) NAPI_METHOD(name)

#define NAPI_MODULE_INIT(name)                                                 \
  void name(napi_env env, napi_value exports, napi_value module, void* priv)


#define CHECK_STATUS                                \
  assert(status == napi_ok);

#define BEGIN(n)                                        \
  napi_status status;                                   \
  napi_value args[n];                                   \
  size_t argc = n;                                      \
  status = napi_get_cb_info(env, info, &argc, args,     \
                            nullptr, nullptr);          \
  CHECK_STATUS;

#endif
