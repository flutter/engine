// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <thread>

#include "C:/Users/tiusic/dev/work/dart-sdk/sdk/runtime/vm/os_thread.h"
#include "C:/Users/tiusic/dev/work/flutter/engine/src/flutter/runtime/dart_isolate.h"
#include "C:/Users/tiusic/dev/work/flutter/engine/src/flutter/runtime/dart_isolate_group_data.h"
#include "C:/Users/tiusic/dev/work/flutter/engine/src/flutter/runtime/isolate_configuration.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/task_runner.h"
#include "flutter/lib/ui/window/platform_isolate.h"

namespace flutter {

void PlatformIsolateNativeApi::Spawn(Dart_Handle entry_point,
                                     Dart_Handle debug_name) {
  const char* debug_name_cstr;
  Dart_StringToCString(debug_name, &debug_name_cstr);
  std::string debug_name_str = debug_name_cstr;
  const char* eps;
  Dart_StringToCString(Dart_ToString(entry_point), &eps);
  std::cout << "PlatformIsolateNativeApi::Spawn\t" << eps << std::endl;

  Dart_Isolate parent_isolate = Dart_CurrentIsolate();
  ASSERT(parent_isolate != NULL);
  std::shared_ptr<DartIsolate>* parent_isolate_data =
      static_cast<std::shared_ptr<DartIsolate>*>(
          Dart_IsolateData(parent_isolate));

  /*global_platform_task_runner->PostTask(
      [parent_isolate, debug_name_str, isolate_send_port_id]() {
        std::cout << "Creating isolate" << std::endl;
        char* error = NULL;
        Dart_Isolate isolate =
            Dart_CreateIsolateInGroup(parent_isolate, debug_name_str.c_str(),
                                      NULL,  // shutdown_callback,
                                      NULL,  // cleanup_callback,
                                      NULL,  // child_isolate_data,
                                      &error);
        Dart_Port new_isolate_main_port_id = Dart_GetMainPortId();
        Dart_Handle new_isolate_main_port =
            Dart_NewSendPort(new_isolate_main_port_id);
        if (!Dart_Post(isolate_send_port_id, new_isolate_main_port)) {
          // TODO: Handle error.
          std::cout << "Failed to send." << std::endl;
        }
      });*/

  Dart_ExitIsolate();  // Exit parent_isolate.

  char* error = NULL;
  // TODO: shutdown_callback and cleanup_callback.
  Dart_Isolate new_isolate =
      Dart_CreateIsolateInGroup(parent_isolate, debug_name_str.c_str(),
                                NULL,  // shutdown_callback,
                                NULL,  // cleanup_callback,
                                NULL,  // child_isolate_data,
                                &error);
  std::cout << "Dart_CreateIsolateInGroup " << (void*)new_isolate << "\t"
            << (error ? error : "null") << std::endl;
  if (error != NULL) {
    // TODO: Handle error.
    std::cout << "Failed to create isolate. " << error << std::endl;
    return;
  }

  void* child_isolate_data;
  bool init_ok =
      DartIsolate::DartIsolateInitializeCallback(&child_isolate_data, &error);
  // TODO: Do we need to set this isolate data on the Isolate somehow, or does
  // DartIsolateInitializeCallback do this? Might need a new API function for
  // this.
  if (!init_ok) {
    // TODO: Handle error.
    std::cout << "Failed to create isolate. " << (error ? error : "no error")
              << std::endl;
    return;
  }

  Dart_EnterScope();

  Dart_PersistentHandle entry_point_handle =
      Dart_NewPersistentHandle(entry_point);

  /*
  Settings settings = global_settings;  // TODO: Where should we get this from ?
  DartIsolate::Flags flags;             // TODO: Set based on snapshot?
  // std::unique_ptr<IsolateConfiguration> isolate_configuration =
  //     IsolateConfiguration::InferFromSettings(settings);  // ???

  std::weak_ptr<DartIsolate> new_isolate =
      DartIsolate::CreateRunningPlatformIsolate(
          settings,
          (*parent_isolate_data)->GetIsolateGroupData().GetIsolateSnapshot(),
          nullptr,  // platform_configuration
          flags,
          nullptr,  // isolate_create_callback
          nullptr,  // isolate_shutdown_callback
          // std::move(isolate_configuration),
          parent_isolate_data->get());
  ASSERT(Dart_CurrentIsolate() != NULL);*/

  /*Dart_Port new_isolate_main_port_id = Dart_GetMainPortId();
  Dart_Handle new_isolate_main_port =
      Dart_NewSendPort(new_isolate_main_port_id);
  if (!Dart_Post(isolate_send_port_id, new_isolate_main_port)) {
    // TODO: Handle error.
    std::cout << "Failed to send." << std::endl;
    return;
  }*/
  Dart_ExitScope();

  Dart_ExitIsolate();  // Exit new_isolate.

  global_platform_task_runner->PostTask([entry_point_handle, new_isolate]() {
    Dart_EnterIsolate(new_isolate);
    Dart_EnterScope();
    std::cout << "Hello from the platform task runner " << GetCurrentThreadId()
              << std::endl;
    Dart_Handle entry_point = Dart_HandleFromPersistent(entry_point_handle);

    std::cout << "Dart_InvokeClosure" << std::endl;
    Dart_InvokeClosure(entry_point, 0, NULL);
    std::cout << "Dart_InvokeClosure done" << std::endl;
    // TODO: Handle error.

    Dart_DeletePersistentHandle(entry_point_handle);
    Dart_ExitScope();
    Dart_ExitIsolate();  // Exit new_isolate.
  });

  Dart_EnterIsolate(parent_isolate);

  // TODO: Do the engine shutdown dance. Destroy all remaining platform isolates
  //     and prevent any more from being spawned. This includes, eg, Android app
  //     backgrounding, where the platform thread is forcibly shut down by the
  //     OS.
  // TODO: How do I avoid including dart_isolate.h? What's the correct API
  //     surface?
}

uint32_t PlatformIsolateNativeApi::GetCurrentThreadId() {
  // return std::hash<std::thread::id>{}(std::this_thread::get_id());
  return dart::OSThread::GetCurrentThreadId();
}

fml::RefPtr<fml::TaskRunner>
    PlatformIsolateNativeApi::global_platform_task_runner;
Settings PlatformIsolateNativeApi::global_settings;

}  // namespace flutter
