// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <thread>

#include "flutter/fml/macros.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_isolate.h"
#include "third_party/tonic/converter/dart_converter.h"

namespace flutter {

void PlatformIsolateNativeApi::Spawn(Dart_Handle entry_point,
                                     Dart_Handle isolate_ready_port,
                                     Dart_Handle debug_name,
                                     bool errors_are_fatal) {
  UIDartState* current_state = UIDartState::Current();
  FML_DCHECK(current_state != nullptr);
  if (!current_state->IsRootIsolate()) {
    // TODO(flutter/flutter#136314): Remove this restriction.
    Dart_EnterScope();
    Dart_ThrowException(tonic::ToDart(
        "PlatformIsolates can only be spawned on the root isolate."));
  }

  Dart_Port isolate_ready_port_id;
  Dart_SendPortGetId(isolate_ready_port, &isolate_ready_port_id);
  const char* debug_name_cstr;
  Dart_StringToCString(debug_name, &debug_name_cstr);

  char* error = nullptr;
  current_state->CreatePlatformIsolate(
      entry_point, isolate_ready_port_id, debug_name_cstr, errors_are_fatal,
      &error);
  if (error) {
    Dart_EnterScope();
    Dart_Handle error_handle = tonic::ToDart<const char*>(error);
    ::free(error);
    Dart_ThrowException(error_handle);
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
