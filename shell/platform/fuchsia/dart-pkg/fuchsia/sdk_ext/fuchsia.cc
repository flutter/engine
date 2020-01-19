// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/dart-pkg/fuchsia/sdk_ext/fuchsia.h"

#include <iterator>  // For std::size
#include <memory>

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle.h"
#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/natives.h"
#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/system.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/converter/dart_converter.h"
#include "flutter/third_party/tonic/dart_class_library.h"
#include "flutter/third_party/tonic/dart_class_provider.h"
#include "flutter/third_party/tonic/logging/dart_error.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace fuchsia::dart {
namespace {

#define REGISTER_FUNCTION(name, count) {"" #name, name, count},
#define DECLARE_FUNCTION(name, count) \
  extern void name(Dart_NativeArguments args);

#define FIDL_NATIVE_LIST(V) V(setReturnCode, 1)

FIDL_NATIVE_LIST(DECLARE_FUNCTION);

struct NativeEntries {
  const char* name;
  Dart_NativeFunction function;
  int argument_count;
} Entries[] = {FIDL_NATIVE_LIST(REGISTER_FUNCTION)};
tonic::DartLibraryNatives* g_natives;

tonic::DartLibraryNatives* InitNatives() {
  tonic::DartLibraryNatives* natives = new tonic::DartLibraryNatives();

  return natives;
}

Dart_NativeFunction NativeLookup(Dart_Handle name,
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
  size_t num_entries = std::size(Entries);
  for (size_t i = 0; i < num_entries; ++i) {
    const struct NativeEntries& entry = Entries[i];
    if (!strcmp(function_name, entry.name) &&
        (entry.argument_count == argument_count)) {
      return entry.function;
    }
  }
  if (!g_natives)
    g_natives = InitNatives();
  return g_natives->GetNativeFunction(name, argument_count, auto_setup_scope);
}

const uint8_t* NativeSymbol(Dart_NativeFunction native_function) {
  size_t num_entries = std::size(Entries);
  for (size_t i = 0; i < num_entries; ++i) {
    const struct NativeEntries& entry = Entries[i];
    if (entry.function == native_function) {
      return reinterpret_cast<const uint8_t*>(entry.name);
    }
  }
  if (!g_natives)
    g_natives = InitNatives();
  return g_natives->GetSymbol(native_function);
}

void setReturnCode(Dart_NativeArguments arguments) {
  int64_t return_code;
  Dart_Handle status =
      Dart_GetNativeIntegerArgument(arguments, 0, &return_code);
  if (!tonic::LogIfError(status)) {
    tonic::DartState::Current()->SetReturnCode(return_code);
  }
}

}  // namespace

void Initialize(
    fidl::InterfaceHandle<fuchsia::sys::Environment> environment,
    fidl::InterfaceRequest<fuchsia::io::Directory> directory_request) {
  zircon::dart::Initialize();

  Dart_Handle library = Dart_LookupLibrary(tonic::ToDart("dart:fuchsia"));
  FX_CHECK(!tonic::LogIfError(library));
  Dart_Handle result = Dart_SetNativeResolver(
      library, fuchsia::dart::NativeLookup, fuchsia::dart::NativeSymbol);
  FX_CHECK(!tonic::LogIfError(result));

  auto dart_state = tonic::DartState::Current();
  auto fuchsia_class_provider =
      std::make_unique<tonic::DartClassProvider>(dart_state, "dart:fuchsia");
  dart_state->class_library().add_provider("fuchsia",
                                           std::move(fuchsia_class_provider));

  auto env_handle = zircon::dart::Handle::Create(environment.TakeChannel());
  result = Dart_SetField(library, tonic::ToDart("_environment"),
                         tonic::ToDart(std::move(env_handle)));
  FX_CHECK(!tonic::LogIfError(result));

  if (directory_request) {
    auto directory_request_handle =
        zircon::dart::Handle::Create(directory_request.TakeChannel());
    result = Dart_SetField(library, tonic::ToDart("_outgoingServices"),
                           tonic::ToDart(std::move(directory_request_handle)));
    FX_CHECK(!tonic::LogIfError(result));
  }
}

}  // namespace fuchsia::dart
