// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <thread>

#include "flutter/fml/macros.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/window/platform_isolate.h"
#include "flutter/runtime/dart_isolate.h"

namespace flutter {

void PlatformIsolateNativeApi::Spawn(Dart_Handle entry_point,
                                     Dart_Handle isolate_ready_port,
                                     Dart_Handle debug_name) {
  Dart_Port isolate_ready_port_id;
  Dart_SendPortGetId(isolate_ready_port, &isolate_ready_port_id);
  const char* debug_name_cstr;
  Dart_StringToCString(debug_name, &debug_name_cstr);
  std::string debug_name_str = debug_name_cstr;
  const char* eps;
  Dart_StringToCString(Dart_ToString(entry_point), &eps);
  std::cout << "PlatformIsolateNativeApi::Spawn\t" << eps << std::endl;

  Dart_Isolate parent_isolate = Dart_CurrentIsolate();
  FML_DCHECK(parent_isolate != NULL);

  Dart_ExitIsolate();  // Exit parent_isolate.

  DartIsolate::CreatePlatformIsolate(parent_isolate, entry_point,
                                     isolate_ready_port_id, debug_name_str);

  Dart_EnterIsolate(parent_isolate);

  // TODO: Do the engine shutdown dance. Destroy all remaining platform isolates
  //     and prevent any more from being spawned. This includes, eg, Android app
  //     backgrounding, where the platform thread is forcibly shut down by the
  //     OS.
  // TODO: How do I avoid including dart_isolate.h? What's the correct API
  //     surface?
}

bool PlatformIsolateNativeApi::IsRunningOnPlatformThread() {
  UIDartState* current_state = UIDartState::Current();
  if (current_state == nullptr) {
    std::cout << "SDFLGKSDLFKGJ 0" << std::endl;
    return false;
  }
  fml::RefPtr<fml::TaskRunner> platform_task_runner =
      current_state->GetTaskRunners().GetPlatformTaskRunner();
  if (!platform_task_runner) {
    std::cout << "SDFLGKSDLFKGJ 1" << std::endl;
    return false;
  }
  return platform_task_runner->RunsTasksOnCurrentThread();
}

}  // namespace flutter
