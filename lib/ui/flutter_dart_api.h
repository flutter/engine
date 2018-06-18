// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_DART_API_H_
#define FLUTTER_LIB_UI_DART_API_H_

#include <cstdarg>

#include "flutter/fml/compiler_specific.h"
#include "third_party/dart/runtime/include/dart_api.h"

#define RETURN_DART_ERROR(args, type, message, ...)                         \
  {                                                                         \
    SetUnhandledExceptionAsReturnValue(args, type, message, ##__VA_ARGS__); \
    return;                                                                 \
  }

#define RETURN_DART_ARGUMENT_ERROR(args, message, ...)                     \
  RETURN_DART_ERROR(args, ::blink::DartErrorType::kArgumentError, message, \
                    ##__VA_ARGS__)

#define RETURN_DART_ENGINE_ERROR(args, message, ...)                     \
  RETURN_DART_ERROR(args, ::blink::DartErrorType::kEngineError, message, \
                    ##__VA_ARGS__)

#define CHECK_AND_RETURN_DART_ERROR(args, arg_to_validate, type, message, ...) \
  {                                                                            \
    if (!::blink::IsHandleValid(arg_to_validate)) {                            \
      RETURN_DART_ERROR(args, type, message, ##__VA_ARGS__);                   \
    }                                                                          \
  };

#define CHECK_AND_RETURN_DART_ARGUMENT_ERROR(args, arg_to_validate, message,   \
                                             ...)                              \
  CHECK_AND_RETURN_DART_ERROR(args, arg_to_validate,                           \
                              ::blink::DartErrorType::kArgumentError, message, \
                              ##__VA_ARGS__)

#define CHECK_AND_RETURN_DART_ENGINE_ERROR(args, arg_to_validate, message,   \
                                           ...)                              \
  CHECK_AND_RETURN_DART_ERROR(args, arg_to_validate,                         \
                              ::blink::DartErrorType::kEngineError, message, \
                              ##__VA_ARGS__)

namespace blink {

enum class DartErrorType {
  // An ArgumentError from dart:core.
  kArgumentError,

  // A string containing the error message.
  // TODO(chinmaygarde): Add a dedicated error type for engine errors.
  kEngineError,
};

bool IsHandleValid(Dart_Handle arg);

void SetUnhandledExceptionAsReturnValue(Dart_NativeArguments args,
                                        DartErrorType type,
                                        const char* message,
                                        ...) FML_PRINTF_FORMAT(3, 4);

}  // namespace blink

#endif  // FLUTTER_LIB_UI_DART_API_H_
