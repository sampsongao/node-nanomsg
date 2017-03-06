#ifndef SRC_MACRO_H
#define SRC_MACRO_H

#define CHECK_STATUS                                \
  if (status != napi_ok) {                          \
    exit(1);                                        \
  }

#define BEGIN(n)                                    \
  napi_status status;                               \
  napi_value args[n];                               \
  status = napi_get_cb_args(env, info, args, n);    \
  CHECK_STATUS;

#endif
