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

bool PlatformIsolateManager::RegisterPlatformIsolate(Dart_Isolate isolate) {
  std::cout << "PlatformIsolateManager::RegisterPlatformIsolate: "
            << (void*)this << "\t" << (void*)isolate << std::endl;
  if (is_shutdown_) {
    std::cout << "           ignored" << std::endl;
    return false;
  }
  std::scoped_lock lock(platform_isolates_lock_);
  if (is_shutdown_) {
    // It's possible shutdown occured while we were trying to aquire the lock.
    std::cout << "           ignored" << std::endl;
    return false;
  }
  FML_DCHECK(platform_isolates_.count(isolate) == 0);
  platform_isolates_.insert(isolate);
  return true;
}

void PlatformIsolateManager::RemovePlatformIsolate(Dart_Isolate isolate) {
  // This method is only called by DartIsolate::OnShutdownCallback() during
  // isolate shutdown. This can happen either during the ordinary platform
  // isolate shutdown, or during ShutdownPlatformIsolates(). In either case
  // we're on the platform thread.
  // TODO: Assert that we're on the platform thread.
  std::cout << "PlatformIsolateManager::RemovePlatformIsolate: " << (void*)this
            << "\t" << (void*)isolate << std::endl;
  if (is_shutdown_) {
    // Removal during ShutdownPlatformIsolates. Ignore, to avoid modifying
    // platform_isolates_ during iteration.
    std::cout << "           ignored" << std::endl;
    return;
  }
  std::scoped_lock lock(platform_isolates_lock_);
  // Since this method and ShutdownPlatformIsolates() are both on the same
  // platform thread, is_shutdown_ can't have changed.
  FML_DCHECK(!is_shutdown_);
  FML_DCHECK(platform_isolates_.count(isolate) != 0);
  platform_isolates_.erase(isolate);
}

void PlatformIsolateManager::ShutdownPlatformIsolates() {
  // TODO: Assert that we're on the platform thread.
  std::cout << "PlatformIsolateManager::ShutdownPlatformIsolates: "
            << (void*)this << "\t" << std::endl;
  std::scoped_lock lock(platform_isolates_lock_);
  is_shutdown_ = true;
  for (Dart_Isolate isolate : platform_isolates_) {
    Dart_EnterIsolate(isolate);
    Dart_ShutdownIsolate();
  }
  platform_isolates_.clear();
}

}  // namespace flutter
