// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/natives.h"

#include <memory>

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle.h"
#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle_waiter.h"
#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/system.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/dart_class_library.h"
#include "flutter/third_party/tonic/dart_class_provider.h"
#include "flutter/third_party/tonic/dart_library_natives.h"
#include "flutter/third_party/tonic/dart_state.h"
#include "flutter/third_party/tonic/logging/dart_error.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace zircon::dart {
namespace {

tonic::DartLibraryNatives* g_natives;

tonic::DartLibraryNatives* InitNatives() {
  tonic::DartLibraryNatives* natives = new tonic::DartLibraryNatives();
  HandleWaiter::RegisterNatives(natives);
  Handle::RegisterNatives(natives);
  System::RegisterNatives(natives);

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
  if (!g_natives)
    g_natives = InitNatives();
  return g_natives->GetNativeFunction(name, argument_count, auto_setup_scope);
}

const uint8_t* NativeSymbol(Dart_NativeFunction native_function) {
  if (!g_natives)
    g_natives = InitNatives();
  return g_natives->GetSymbol(native_function);
}

}  // namespace

void Initialize() {
  Dart_Handle library = Dart_LookupLibrary(tonic::ToDart("dart:zircon"));
  FX_CHECK(!tonic::LogIfError(library));
  Dart_Handle result = Dart_SetNativeResolver(
      library, zircon::dart::NativeLookup, zircon::dart::NativeSymbol);
  FX_CHECK(!tonic::LogIfError(result));

  auto dart_state = tonic::DartState::Current();
  auto zircon_class_provider =
      std::make_unique<tonic::DartClassProvider>(dart_state, "dart:zircon");
  dart_state->class_library().add_provider("zircon",
                                           std::move(zircon_class_provider));
}

}  // namespace zircon::dart
