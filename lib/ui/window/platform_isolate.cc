// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <thread>

#include "flutter/fml/macros.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_isolate.h"

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
  std::cout << "PlatformIsolateNativeApi::Spawn\t" << debug_name_str << "\t"
            << eps << std::endl;

  UIDartState* current_state = UIDartState::Current();
  FML_DCHECK(current_state != nullptr);

  Dart_Isolate platform_isolate = current_state->CreatePlatformIsolate(
      entry_point, isolate_ready_port_id, debug_name_str);
  if (platform_isolate == nullptr) {
    // TODO: Report error to Dart.
  }
}

bool PlatformIsolateNativeApi::IsRunningOnPlatformThread() {
  UIDartState* current_state = UIDartState::Current();
  if (current_state == nullptr) {
    return false;
  }
  fml::RefPtr<fml::TaskRunner> platform_task_runner =
      current_state->GetTaskRunners().GetPlatformTaskRunner();
  if (!platform_task_runner) {
    return false;
  }
  return platform_task_runner->RunsTasksOnCurrentThread();
}

}  // namespace flutter
