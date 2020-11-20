// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_deferred_load_handler.h"

#include "flutter/runtime/runtime_controller.h"

namespace flutter {

void* DartDeferredLoadHandler::runtime_controller_ = nullptr;

Dart_DeferredLoadHandler
    DartDeferredLoadHandler::empty_dart_deferred_load_handler =
        &DartDeferredLoadHandler::EmptyDartLoadLibrary;

Dart_DeferredLoadHandler DartDeferredLoadHandler::dart_deferred_load_handler =
    &DartDeferredLoadHandler::OnDartLoadLibrary;

void DartDeferredLoadHandler::RegisterRuntimeController(
    void* runtime_controller) {
  runtime_controller_ = runtime_controller;
}

Dart_Handle DartDeferredLoadHandler::OnDartLoadLibrary(
    intptr_t loading_unit_id) {
  if (runtime_controller_ != nullptr)
    return static_cast<RuntimeController*>(runtime_controller_)
        ->OnDartLoadLibrary(loading_unit_id);
  return Dart_Null();
}

Dart_Handle DartDeferredLoadHandler::EmptyDartLoadLibrary(
    intptr_t loading_unit_id) {
  return Dart_Null();
}

}  // namespace flutter
