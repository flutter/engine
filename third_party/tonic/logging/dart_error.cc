// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tonic/logging/dart_error.h"

#include "tonic/common/macros.h"
#include "tonic/converter/dart_converter.h"

namespace tonic {
namespace DartError {
const char kInvalidArgument[] = "Invalid argument.";
DartPersistentValue on_error;
UnhandledExceptionReporter log_unhandled_exception = [](const std::string&,
                                                        const std::string&) {};

void ReportUnhandledException(Dart_Handle exception_handle,
                              Dart_Handle stack_trace_handle) {
  const std::string exception =
      tonic::StdStringFromDart(Dart_ToString(exception_handle));
  const std::string stack_trace =
      tonic::StdStringFromDart(Dart_ToString(stack_trace_handle));
  DartError::log_unhandled_exception(exception, stack_trace);
}
}  // namespace DartError

void SetErrorHandler(DartState* dart_state, Dart_Handle value) {
  DartError::on_error.Clear();
  if (Dart_IsNull(value)) {
    return;
  }
  DartError::on_error.Set(dart_state, value);
}

void SetUnhandledExceptionReporter(
    DartError::UnhandledExceptionReporter reporter) {
  DartError::log_unhandled_exception = reporter;
}

bool CheckAndHandleError(Dart_Handle handle) {
  // Specifically handle UnhandledExceptionErrors first. These exclude fatal
  // errors that are shutting down the vm and compilation errors in source code.
  if (Dart_IsUnhandledExceptionError(handle)) {
    Dart_Handle exception_handle = Dart_ErrorGetException(handle);
    Dart_Handle stack_trace_handle = Dart_ErrorGetStackTrace(handle);
    Dart_Handle on_error = DartError::on_error.Get();

    // Hooks.dart will call the error handler on PlatformDispatcher if it is
    // not null. If it is null, returns false, fall into the !handled branch
    // below and log.
    // If it is not null, defer to the return value of that closure
    // to determine whether to report via logging.
    bool handled = false;
    if (on_error) {
      Dart_Handle args[2];
      args[0] = exception_handle;
      args[1] = stack_trace_handle;
      Dart_Handle on_error_result = Dart_InvokeClosure(on_error, 2, args);

      // An exception was thrown by the exception handler.
      if (Dart_IsError(on_error_result)) {
        DartError::ReportUnhandledException(
            Dart_ErrorGetException(on_error_result),
            Dart_ErrorGetStackTrace(on_error_result));
        handled = false;
      } else {
        handled = DartConverter<bool>::FromDart(on_error_result);
      }
    }
    if (!handled) {
      DartError::ReportUnhandledException(exception_handle, stack_trace_handle);
    }
    return true;
  } else if (Dart_IsError(handle)) {
    tonic::Log("Dart Error: %s", Dart_GetError(handle));
    return true;
  } else {
    return false;
  }
}

DartErrorHandleType GetErrorHandleType(Dart_Handle handle) {
  if (Dart_IsCompilationError(handle)) {
    return kCompilationErrorType;
  } else if (Dart_IsApiError(handle)) {
    return kApiErrorType;
  } else if (Dart_IsError(handle)) {
    return kUnknownErrorType;
  } else {
    return kNoError;
  }
}

int GetErrorExitCode(Dart_Handle handle) {
  if (Dart_IsCompilationError(handle)) {
    return 254;  // dart::bin::kCompilationErrorExitCode
  } else if (Dart_IsApiError(handle)) {
    return 253;  // dart::bin::kApiErrorExitCode
  } else if (Dart_IsError(handle)) {
    return 255;  // dart::bin::kErrorExitCode
  } else {
    return 0;
  }
}

}  // namespace tonic
