// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_TONIC_LOGGING_DART_ERROR_H_
#define LIB_TONIC_LOGGING_DART_ERROR_H_

#include "third_party/dart/runtime/include/dart_api.h"

#include "tonic/dart_persistent_value.h"

namespace tonic {

namespace DartError {
using UnhandledExceptionReporter =
    std::function<void(const std::string&, const std::string&)>;

extern const char kInvalidArgument[];
extern DartPersistentValue on_error;
extern UnhandledExceptionReporter log_unhandled_exception;
}  // namespace DartError

/// Check if a Dart_Handle is an error or exception.
///
/// If it is an error or exception, this method will return true.
///
/// If it is an unhandled error or exception, first a call will be made to the
/// Dart code set in |SetErrorHandler|. If that closure returns false, a
/// fallback is made to the closure in |SetUnhandledExceptionReporter| to log
/// details of the exception and stack. If that closure throws an exception
/// itself, the exception and stack are logged via the fallback method, and the
/// original exception and stack are also logged via the fallback method.
///
/// The fallback behavior matches the behavior of Flutter applications before
/// the introduction of PlatformDispatcher.onError.
///
/// If it is not an unhandled exception, it is logged to stderr or some similar
/// mechanism provided by the platform. Depending on which type of error occurs,
/// the process may crash and the Dart isolate may be unusable. Errors that fall
/// into this category include compilation errors, Dart API errors, and unwind
/// errors that will terminate the Dart VM.
///
/// Historically known as LogIfError.
bool CheckAndHandleError(Dart_Handle handle);

/// Provides a Dart_Handle to a top level static field closure to invoke when
/// an unhandled exception occurs.
///
/// The signature of this field must match that of _onError in hooks.dart,
/// namely `bool _onError(Object error, StackTrace? stackTrace)`.
///
/// This handler will be invoked when an unhandled error occurs. If it returns
/// false or throws an exception, a fallback handler will be invoked. By
/// default, UIDartState registers its ReportUnhandledError and will either call
/// a closure provided in the Shell Settings or simply dump the error to stderr
/// or some similar platform specific mechanism.
void SetErrorHandler(DartState* dart_state, Dart_Handle value);

/// The fallback mechanism to log errors if |SetErrorHandler| has provided a
/// null value or a closure that returns false.
///
/// Normally, it UIDartState registers with this method in its constructor.
void SetUnhandledExceptionReporter(
    DartError::UnhandledExceptionReporter reporter);

enum DartErrorHandleType {
  kNoError,
  kUnknownErrorType,
  kApiErrorType,
  kCompilationErrorType,
};

DartErrorHandleType GetErrorHandleType(Dart_Handle handle);

int GetErrorExitCode(Dart_Handle handle);

}  // namespace tonic

#endif  // LIB_TONIC_DART_ERROR_H_
