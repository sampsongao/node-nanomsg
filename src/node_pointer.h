// Source: @tootallnate https://gist.github.com/TooTallNate/3987725
// Maintain an up-to-date copy of this file when necessary.

#pragma once

#include <node_jsvmapi.h>


/*
 * Wraps "ptr" into a new SlowBuffer instance with size "length".
 */

inline static napi_value WrapPointer(void *ptr, size_t length) {
  napi_env env = napi_get_current_env();
  return napi_buffer_copy(env, static_cast<char *>(ptr), length);
}

/*
 * Wraps "ptr" into a new SlowBuffer instance with length 0.
 */

inline static napi_value WrapPointer(void *ptr) {
  return WrapPointer(ptr, 0);
}

/*
 * Unwraps Buffer instance "buffer" to a C `char *` with the offset specified.
 */

inline static char *UnwrapPointer(napi_value buffer, int64_t offset) {
  napi_env env = napi_get_current_env();
  if (napi_buffer_has_instance(env, buffer)) {
    return napi_buffer_data(env, buffer) + offset;
  } else {
    return 0;
  }
}

/*
 * Unwraps Buffer instance "buffer" to a C `char *` (no offset applied).
 */

inline static char *UnwrapPointer(napi_value buffer) {
  napi_env env = napi_get_current_env();
  if (napi_buffer_has_instance(env, buffer)) {
    return napi_buffer_data(env, buffer);
  } else {
    return 0;
  }
}

/**
 * Templated version of UnwrapPointer that does a reinterpret_cast() on the
 * pointer before returning it.
 */

template <typename Type>
inline static Type UnwrapPointer(napi_value buffer) {
  return reinterpret_cast<Type>(UnwrapPointer(buffer));
}
