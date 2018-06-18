// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/flutter_dart_api.h"

#include <cstdio>

#ifdef _DEBUG
// Needed by dartutils.h below.
#define DEBUG
#endif  // _DEBUG

#include "third_party/dart/runtime/bin/dartutils.h"
#include "topaz/lib/tonic/converter/dart_converter.h"

namespace blink {

static Dart_Handle ExceptionValueForType(DartErrorType type,
                                         const char* format,
                                         va_list message_args) {
  char message_buffer[256];
  vsnprintf(message_buffer, sizeof(message_buffer), format, message_args);
  switch (type) {
    case DartErrorType::kArgumentError:
      return ::dart::bin::DartUtils::NewDartArgumentError(message_buffer);
    case DartErrorType::kEngineError:
      // TODO(chinmaygarde): Placeholder till the engine gets its own error
      // type.
      return Dart_NewStringFromCString(message_buffer);
  }

  return Dart_Null();
}

void SetUnhandledExceptionAsReturnValue(Dart_NativeArguments args,
                                        DartErrorType type,
                                        const char* message,
                                        ...) {
  va_list message_args;
  va_start(message_args, message);
  auto exception_value = ExceptionValueForType(type, message, message_args);
  va_end(message_args);

  auto unhandled_exception = Dart_NewUnhandledExceptionError(exception_value);
  Dart_SetReturnValue(args, unhandled_exception);
}

bool IsHandleValid(Dart_Handle arg) {
  return arg != nullptr && !Dart_IsError(arg);
}

}  // namespace blink
