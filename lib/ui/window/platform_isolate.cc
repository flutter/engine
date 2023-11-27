// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <thread>

#include "C:/Users/tiusic/dev/work/dart-sdk/sdk/runtime/vm/os_thread.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/window/platform_isolate.h"

namespace flutter {

// Dart_Handle PlatformIsolateNativeApi::Spawn(
//     Dart_Handle entry_point, Dart_Handle message) {
//   DartIsolate::CreateRunningPlatformIsolate();
// }

uint32_t PlatformIsolateNativeApi::GetCurrentThreadId() {
  // return std::hash<std::thread::id>{}(std::this_thread::get_id());
  return dart::OSThread::GetCurrentThreadId();
}

fml::RefPtr<fml::TaskRunner>
    PlatformIsolateNativeApi::global_platform_task_runner;

}  // namespace flutter
