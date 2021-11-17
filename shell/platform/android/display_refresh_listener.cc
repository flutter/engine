// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display_refresh_listener.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/native_library.h"

namespace flutter {
namespace {
// Only avialalbe on API 24+
typedef void AChoreographer;

// Must be called from the Platform thread.
AChoreographer* GetAChoreographer() {
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(libandroid);
  auto get_instance_fn = libandroid->ResolveFunction<AChoreographer* (*)(void)>(
      "AChoreographer_getInstance");
  if (get_instance_fn) {
    return get_instance_fn.value()();
  }
  return nullptr;
}

// Only available on API 30+
typedef void (*AChoreographer_refreshRateCallback)(int64_t vsyncPeriodNanos,
                                                   void* data);
void OnDisplayRefreshUpdated(int64_t vsyncPeriodNanos, void* data) {
  auto* delegate = reinterpret_cast<DisplayRefreshListener::Delegate*>(data);
  delegate->OnDisplayRefreshUpdated(vsyncPeriodNanos);
}

// The data parameter will not be accessed, but will serve as an identifier for
// unregistering.
void ListenToRefreshRateChanges(DisplayRefreshListener::Delegate* delegate) {
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(libandroid);
  auto register_refresh_rate_callback_fn = libandroid->ResolveFunction<void (*)(
      AChoreographer*, AChoreographer_refreshRateCallback, void*)>(
      "AChoreographer_registerRefreshRateCallback");

  if (register_refresh_rate_callback_fn) {
    auto choreographer = GetAChoreographer();
    FML_CHECK(choreographer);
    register_refresh_rate_callback_fn.value()(
        choreographer, &OnDisplayRefreshUpdated, delegate);
  }
}

// The data parmaeter must be the same as the pointer passed to
// ListenToRefreshRateChanges.
void StopListeningToRefreshRateChanges(
    DisplayRefreshListener::Delegate* delegate) {
  auto libandroid = fml::NativeLibrary::Create("libandroid.so");
  FML_CHECK(libandroid);
  auto unregister_refresh_rate_callback_fn =
      libandroid->ResolveFunction<void (*)(
          AChoreographer*, AChoreographer_refreshRateCallback, void*)>(
          "AChoreographer_unregisterRefreshRateCallback");

  if (unregister_refresh_rate_callback_fn) {
    auto choreographer = GetAChoreographer();
    FML_CHECK(choreographer);
    unregister_refresh_rate_callback_fn.value()(
        choreographer, &OnDisplayRefreshUpdated, delegate);
  }
}
}  // namespace

DisplayRefreshListener::DisplayRefreshListener(Delegate& delegate)
    : delegate_(delegate) {
  ListenToRefreshRateChanges(&delegate_);
}

DisplayRefreshListener::~DisplayRefreshListener() {
  StopListeningToRefreshRateChanges(&delegate_);
}
}  // namespace flutter
