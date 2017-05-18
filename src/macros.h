#ifndef SRC_MACRO_H
#define SRC_MACRO_H
#include <assert.h>

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
