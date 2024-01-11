// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_PLATFORM_ISOLATE_MANAGER_H_
#define FLUTTER_RUNTIME_PLATFORM_ISOLATE_MANAGER_H_

#include <atomic>
#include <mutex>
#include <unordered_set>

#include "third_party/dart/runtime/include/dart_api.h"

namespace flutter {

class PlatformIsolateManager {
 public:
  bool IsShutdown() const { return is_shutdown_; }
  bool RegisterPlatformIsolate(Dart_Isolate isolate);
  void RemovePlatformIsolate(Dart_Isolate isolate);
  void ShutdownPlatformIsolates();

  // For testing only.
  bool IsRegistered(Dart_Isolate isolate);

 private:
  std::mutex platform_isolates_lock_;
  std::unordered_set<Dart_Isolate> platform_isolates_;
  std::atomic<bool> is_shutdown_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_RUNTIME_PLATFORM_ISOLATE_MANAGER_H_
