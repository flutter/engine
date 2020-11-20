// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_DART_DEFERRED_LOAD_HANDLER_H_
#define FLUTTER_RUNTIME_DART_DEFERRED_LOAD_HANDLER_H_

#include <memory>
#include <string>

#include "third_party/dart/runtime/include/dart_api.h"

namespace flutter {

class DartDeferredLoadHandler {
 public:
  // TODO: Fix deps to not use void*
  static void RegisterRuntimeController(void* runtime_controller);

  static Dart_DeferredLoadHandler empty_dart_deferred_load_handler;
  static Dart_DeferredLoadHandler dart_deferred_load_handler;

 private:
  static Dart_Handle OnDartLoadLibrary(intptr_t loading_unit_id);
  // No-op function that returns Dart_Null() for when the isolate is not
  // expected to handle deferred libraries.
  static Dart_Handle EmptyDartLoadLibrary(intptr_t loading_unit_id);
  static void* runtime_controller_;
};

}  // namespace flutter

#endif  // FLUTTER_RUNTIME_DART_DEFERRED_LOAD_HANDLER_H_
