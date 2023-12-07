// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/platform_isolate_manager.h"

#include "flutter/runtime/dart_isolate.h"

#include <iostream>

namespace flutter {

PlatformIsolateManager::PlatformIsolateManager() {
  std::cout << "PlatformIsolateManager constructor: " << (void*)this
            << std::endl;
}

void PlatformIsolateManager::RegisterPlatformIsolate(Dart_Isolate isolate) {
  FML_DCHECK(!is_shutdown_);
  std::cout << "PlatformIsolateManager::RegisterPlatformIsolate: "
            << (void*)this << "\t" << (void*)isolate << std::endl;
  std::scoped_lock lock(platform_isolates_lock_);
  FML_DCHECK(platform_isolates_.count(isolate) == 0);
  platform_isolates_.insert(isolate);
}

void PlatformIsolateManager::RemovePlatformIsolate(Dart_Isolate isolate) {
  std::cout << "PlatformIsolateManager::RemovePlatformIsolate: " << (void*)this
            << "\t" << (void*)isolate << std::endl;
  if (is_shutdown_) {
    // Removal during ShutdownPlatformIsolates. Ignore, to avoid modifying
    // platform_isolates_ during iteration.
    std::cout << "           ignored" << std::endl;
    return;
  }
  std::scoped_lock lock(platform_isolates_lock_);
  FML_DCHECK(platform_isolates_.count(isolate) != 0);
  platform_isolates_.erase(isolate);
}

void PlatformIsolateManager::ShutdownPlatformIsolates() {
  // TODO: Assert that we're on the platform thread.
  std::cout << "PlatformIsolateManager::ShutdownPlatformIsolates: "
            << (void*)this << "\t" << std::endl;
  is_shutdown_ = true;
  std::scoped_lock lock(platform_isolates_lock_);
  for (Dart_Isolate isolate : platform_isolates_) {
    Dart_EnterIsolate(isolate);
    Dart_ShutdownIsolate();
  }
  platform_isolates_.clear();
}

}  // namespace flutter
