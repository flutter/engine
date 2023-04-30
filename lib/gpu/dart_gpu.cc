// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/dart_gpu.h"

#include <mutex>
#include <string_view>

#include "flutter/common/settings.h"
#include "flutter/fml/build_config.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/logging/dart_error.h"

using tonic::ToDart;

namespace flutter {

#define FFI_FUNCTION_LIST(V) /* Constructors */

#define FFI_METHOD_LIST(V) /* Methods */

#define FFI_FUNCTION_INSERT(FUNCTION, ARGS)     \
  g_function_dispatchers.insert(std::make_pair( \
      std::string_view(#FUNCTION),              \
      reinterpret_cast<void*>(                  \
          tonic::FfiDispatcher<void, decltype(&FUNCTION), &FUNCTION>::Call)));

#define FFI_METHOD_INSERT(CLASS, METHOD, ARGS)                                 \
  g_function_dispatchers.insert(                                               \
      std::make_pair(std::string_view(#CLASS "::" #METHOD),                    \
                     reinterpret_cast<void*>(                                  \
                         tonic::FfiDispatcher<CLASS, decltype(&CLASS::METHOD), \
                                              &CLASS::METHOD>::Call)));

namespace {

std::once_flag g_dispatchers_init_flag;
std::unordered_map<std::string_view, void*> g_function_dispatchers;

void* ResolveFfiNativeFunction(const char* name, uintptr_t args) {
  auto it = g_function_dispatchers.find(name);
  return (it != g_function_dispatchers.end()) ? it->second : nullptr;
}

void InitDispatcherMap() {
  FFI_FUNCTION_LIST(FFI_FUNCTION_INSERT)
  FFI_METHOD_LIST(FFI_METHOD_INSERT)
}

}  // anonymous namespace

void DartGPU::InitForIsolate(const Settings& settings) {
  if (!settings.enable_impeller) {
    return;
  }
  std::call_once(g_dispatchers_init_flag, InitDispatcherMap);

  auto dart_gpu = Dart_LookupLibrary(ToDart("dart:gpu"));
  if (Dart_IsError(dart_gpu)) {
    Dart_PropagateError(dart_gpu);
  }

  // Set up FFI Native resolver for dart:ui.
  Dart_Handle result =
      Dart_SetFfiNativeResolver(dart_gpu, ResolveFfiNativeFunction);
  if (Dart_IsError(result)) {
    Dart_PropagateError(result);
  }
}

}  // namespace flutter
