// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tonic/dart_args.h"

namespace tonic {

intptr_t FormatFfiNativeAnnotation(char* buf,
                                   intptr_t buf_size,
                                   const char* ret_type,
                                   const char* name,
                                   bool has_self,
                                   size_t n_args,
                                   va_list args) {
  intptr_t written = 0;

  written += snprintf(buf, buf_size, "@FfiNative<%s Function(", ret_type);
  if (has_self) {
    written += snprintf((buf == nullptr ? nullptr : buf + written),
                        (buf_size == 0 ? 0 : (buf_size - written)), "Pointer");
  }

  for (size_t i = 0; i < n_args; ++i) {
    if (i > 0 || has_self) {
      written += snprintf((buf == nullptr ? nullptr : buf + written),
                          (buf_size == 0 ? 0 : (buf_size - written)), ", ");
    }
    written += snprintf((buf == nullptr ? nullptr : buf + written),
                        (buf_size == 0 ? 0 : (buf_size - written)), "%s",
                        va_arg(args, const char*));
  }

  written +=
      snprintf((buf == nullptr ? nullptr : buf + written),
               (buf_size == 0 ? 0 : (buf_size - written)), ")>(\"%s\")", name);

  return written;
}

std::unique_ptr<char[]> FormatFfiNativeAnnotation(const char* ret_type,
                                                  const char* name,
                                                  bool has_self,
                                                  size_t n_args,
                                                  ...) {
  va_list args;
  va_start(args, n_args);
  // Determine length.
  intptr_t len = FormatFfiNativeAnnotation(nullptr, 0, ret_type, name, has_self,
                                           n_args, args) +
                 1;
  // Allocate and construct string.
  std::unique_ptr<char[]> message(new char[len]);
  FormatFfiNativeAnnotation(message.get(), len, ret_type, name, has_self,
                            n_args, args);
  va_end(args);
  return message;
}

intptr_t FormatFfiNativeFunction(char* buf,
                                 intptr_t buf_size,
                                 const char* ret_type,
                                 const char* name,
                                 bool has_self,
                                 size_t n_args,
                                 va_list args) {
  intptr_t written = 0;

  // TODO(cskau): We could insert name instead of FUNCTION.
  written += snprintf(buf, buf_size, "external static %s FUNCTION(", ret_type);
  if (has_self) {
    written +=
        snprintf((buf == nullptr ? nullptr : buf + written),
                 (buf_size == 0 ? 0 : (buf_size - written)), "Pointer self");
  }

  for (size_t i = 0; i < n_args; ++i) {
    if (has_self || i > 0) {
      written += snprintf((buf == nullptr ? nullptr : buf + written),
                          (buf_size == 0 ? 0 : (buf_size - written)), ", ");
    }
    written += snprintf((buf == nullptr ? nullptr : buf + written),
                        (buf_size == 0 ? 0 : (buf_size - written)), "%s ARG",
                        va_arg(args, const char*));
  }

  written += snprintf((buf == nullptr ? nullptr : buf + written),
                      (buf_size == 0 ? 0 : (buf_size - written)), ")");

  return written;
}

std::unique_ptr<char[]> FormatFfiNativeFunction(const char* ret_type,
                                                const char* name,
                                                bool has_self,
                                                size_t n_args,
                                                ...) {
  va_list args;
  va_start(args, n_args);
  // Determine length.
  intptr_t len = FormatFfiNativeFunction(nullptr, 0, ret_type, name, has_self,
                                         n_args, args) +
                 1;
  // Allocate and construct string.
  std::unique_ptr<char[]> message(new char[len]);
  FormatFfiNativeFunction(message.get(), len, ret_type, name, has_self, n_args,
                          args);
  va_end(args);
  return message;
}

}  // namespace tonic
