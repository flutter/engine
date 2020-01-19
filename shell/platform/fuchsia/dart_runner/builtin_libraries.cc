// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/dart_runner/builtin_libraries.h"

#include <iterator>  // For std::size
#include <optional>

#include "flutter/shell/platform/fuchsia/dart-pkg/fuchsia/sdk_ext/fuchsia.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/converter/dart_converter.h"
#include "flutter/third_party/tonic/dart_microtask_queue.h"
#include "flutter/third_party/tonic/logging/dart_error.h"
#include "third_party/dart/runtime/bin/io_natives.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace dart_runner {
namespace {

#define REGISTER_FUNCTION(name, count) {#name, name, count},
#define DECLARE_FUNCTION(name, count) \
  extern void name(Dart_NativeArguments args);

#define FOR_EACH_BINDING(V) \
  V(loggerPrintString, 1)   \
  V(scheduleMicrotask, 1)

FOR_EACH_BINDING(DECLARE_FUNCTION);

const struct NativeEntry {
  const char* name;
  Dart_NativeFunction function;
  int argument_count;
} kBuiltinEntries[] = {FOR_EACH_BINDING(REGISTER_FUNCTION)};

Dart_NativeFunction BuiltinNativeLookup(Dart_Handle name,
                                        int argument_count,
                                        bool* auto_setup_scope) {
  const char* function_name = nullptr;
  Dart_Handle result = Dart_StringToCString(name, &function_name);
  if (Dart_IsError(result)) {
    Dart_PropagateError(result);
  }
  FX_DCHECK(function_name != nullptr);
  FX_DCHECK(auto_setup_scope != nullptr);
  *auto_setup_scope = true;
  size_t num_entries = std::size(kBuiltinEntries);
  for (size_t i = 0; i < num_entries; i++) {
    const NativeEntry& entry = kBuiltinEntries[i];
    if (!strcmp(function_name, entry.name) &&
        (entry.argument_count == argument_count)) {
      return entry.function;
    }
  }
  return nullptr;
}

const uint8_t* BuiltinNativeSymbol(Dart_NativeFunction native_function) {
  size_t num_entries = std::size(kBuiltinEntries);
  for (size_t i = 0; i < num_entries; i++) {
    const NativeEntry& entry = kBuiltinEntries[i];
    if (entry.function == native_function)
      return reinterpret_cast<const uint8_t*>(entry.name);
  }
  return nullptr;
}

void loggerPrintString(Dart_NativeArguments args) {
  intptr_t length = 0;
  uint8_t* chars = nullptr;
  Dart_Handle str = Dart_GetNativeArgument(args, 0);
  Dart_Handle result = Dart_StringToUTF8(str, &chars, &length);
  if (Dart_IsError(result)) {
    Dart_PropagateError(result);
  } else {
    fwrite(chars, 1, length, stdout);
    fputc('\n', stdout);
    fflush(stdout);
  }
}

void scheduleMicrotask(Dart_NativeArguments args) {
  Dart_Handle closure = Dart_GetNativeArgument(args, 0);
  if (tonic::LogIfError(closure) || !Dart_IsClosure(closure))
    return;
  tonic::DartMicrotaskQueue::GetForCurrentThread()->ScheduleMicrotask(closure);
}

}  // namespace

void InitBuiltinLibraries(
    const std::string& script_uri,
    fdio_ns_t* namespc,
    fidl::InterfaceHandle<fuchsia::sys::Environment> environment,
    fidl::InterfaceRequest<fuchsia::io::Directory> directory_request,
    int stdoutfd,
    int stderrfd,
    bool service_isolate) {
  // dart:fuchsia --------------------------------------------------------------

  if (!service_isolate) {
    fuchsia::dart::Initialize(std::move(environment),
                              std::move(directory_request));
  }

  // dart:fuchsia.builtin ------------------------------------------------------

  Dart_Handle builtin_lib =
      Dart_LookupLibrary(tonic::ToDart("dart:fuchsia.builtin"));
  FX_CHECK(!tonic::LogIfError(builtin_lib));
  Dart_Handle result = Dart_SetNativeResolver(builtin_lib, BuiltinNativeLookup,
                                              BuiltinNativeSymbol);
  FX_CHECK(!tonic::LogIfError(result));

  // dart:io -------------------------------------------------------------------

  Dart_Handle io_lib = Dart_LookupLibrary(tonic::ToDart("dart:io"));
  FX_CHECK(!tonic::LogIfError(io_lib));
  result = Dart_SetNativeResolver(io_lib, dart::bin::IONativeLookup,
                                  dart::bin::IONativeSymbol);
  FX_CHECK(!tonic::LogIfError(result));

  // dart:zircon ---------------------------------------------------------------

  Dart_Handle zircon_lib = Dart_LookupLibrary(tonic::ToDart("dart:zircon"));
  FX_CHECK(!tonic::LogIfError(zircon_lib));
  // NativeResolver already set by fuchsia::dart::Initialize().

  // Core libraries ------------------------------------------------------------

  Dart_Handle async_lib = Dart_LookupLibrary(tonic::ToDart("dart:async"));
  FX_CHECK(!tonic::LogIfError(async_lib));

  Dart_Handle core_lib = Dart_LookupLibrary(tonic::ToDart("dart:core"));
  FX_CHECK(!tonic::LogIfError(core_lib));

  Dart_Handle internal_lib =
      Dart_LookupLibrary(tonic::ToDart("dart:_internal"));
  FX_CHECK(!tonic::LogIfError(internal_lib));

  Dart_Handle isolate_lib = Dart_LookupLibrary(tonic::ToDart("dart:isolate"));
  FX_CHECK(!tonic::LogIfError(isolate_lib));

#if !defined(AOT_RUNTIME)
  // AOT: These steps already happened at compile time in gen_snapshot.

  // We need to ensure that all the scripts loaded so far are finalized
  // as we are about to invoke some Dart code below to setup closures.
  result = Dart_FinalizeLoading(false);
  FX_CHECK(!tonic::LogIfError(result));
#endif

  // Setup the internal library's 'internalPrint' function.
  Dart_Handle print =
      Dart_Invoke(builtin_lib, tonic::ToDart("_getPrintClosure"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(print));

  result = Dart_SetField(internal_lib, tonic::ToDart("_printClosure"), print);
  FX_CHECK(!tonic::LogIfError(result));

  // Set up the 'scheduleImmediate' closure.
  Dart_Handle schedule_immediate_closure;
  if (service_isolate) {
    // Running on dart::ThreadPool.
    schedule_immediate_closure = Dart_Invoke(
        isolate_lib, tonic::ToDart("_getIsolateScheduleImmediateClosure"), 0,
        nullptr);
  } else {
    // Running on async::Loop.
    schedule_immediate_closure = Dart_Invoke(
        builtin_lib, tonic::ToDart("_getScheduleMicrotaskClosure"), 0, nullptr);
  }
  FX_CHECK(!tonic::LogIfError(schedule_immediate_closure));

  Dart_Handle schedule_args[1];
  schedule_args[0] = schedule_immediate_closure;
  result = Dart_Invoke(async_lib, tonic::ToDart("_setScheduleImmediateClosure"),
                       1, schedule_args);
  FX_CHECK(!tonic::LogIfError(result));

  // Set up the namespace in dart:io.
  Dart_Handle namespace_type =
      Dart_GetType(io_lib, tonic::ToDart("_Namespace"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(namespace_type));

  Dart_Handle namespace_args[1];
  namespace_args[0] = tonic::ToDart(reinterpret_cast<intptr_t>(namespc));
  result = Dart_Invoke(namespace_type, tonic::ToDart("_setupNamespace"), 1,
                       namespace_args);
  FX_CHECK(!tonic::LogIfError(result));

  // Set up the namespace in dart:zircon.
  namespace_type =
      Dart_GetType(zircon_lib, tonic::ToDart("_Namespace"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(namespace_type));

  result = Dart_SetField(namespace_type, tonic::ToDart("_namespace"),
                         tonic::ToDart(reinterpret_cast<intptr_t>(namespc)));
  FX_CHECK(!tonic::LogIfError(result));

  // Set up stdout and stderr.
  Dart_Handle stdio_args[3];
  stdio_args[0] = Dart_NewInteger(0);
  stdio_args[1] = Dart_NewInteger(stdoutfd);
  stdio_args[2] = Dart_NewInteger(stderrfd);
  result = Dart_Invoke(io_lib, tonic::ToDart("_setStdioFDs"), 3, stdio_args);
  FX_CHECK(!tonic::LogIfError(result));

  // Disable some dart:io operations.
  Dart_Handle embedder_config_type =
      Dart_GetType(io_lib, tonic::ToDart("_EmbedderConfig"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(embedder_config_type));

  result = Dart_SetField(embedder_config_type, tonic::ToDart("_mayExit"),
                         Dart_False());
  FX_CHECK(!tonic::LogIfError(result));

  // Set the script location.
  result = Dart_SetField(builtin_lib, tonic::ToDart("_rawScript"),
                         tonic::ToDart(script_uri));
  FX_CHECK(!tonic::LogIfError(result));

  // Setup the uriBase with the base uri of the fidl app.
  Dart_Handle uri_base =
      Dart_Invoke(io_lib, tonic::ToDart("_getUriBaseClosure"), 0, nullptr);
  FX_CHECK(!tonic::LogIfError(uri_base));

  result = Dart_SetField(core_lib, tonic::ToDart("_uriBaseClosure"), uri_base);
  FX_CHECK(!tonic::LogIfError(result));

  Dart_Handle setup_hooks = tonic::ToDart("_setupHooks");
  result = Dart_Invoke(builtin_lib, setup_hooks, 0, nullptr);
  FX_CHECK(!tonic::LogIfError(result));
  result = Dart_Invoke(io_lib, setup_hooks, 0, nullptr);
  FX_CHECK(!tonic::LogIfError(result));
  result = Dart_Invoke(isolate_lib, setup_hooks, 0, nullptr);
  FX_CHECK(!tonic::LogIfError(result));
}

}  // namespace dart_runner
