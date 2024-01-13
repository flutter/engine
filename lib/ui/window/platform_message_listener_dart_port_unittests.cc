// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/common/task_runners.h"
#include "flutter/fml/mapping.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/lib/ui/window/platform_message_listener_dart_port.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell_test.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"

namespace flutter {
namespace testing {

TEST_F(ShellTest, PlatformListenerDartPort) {
  bool did_pass = false;
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  auto nativeCallPlatformListenerDartPort =
      [ui_task_runner =
           task_runners.GetUITaskRunner()](Dart_NativeArguments args) {
        auto dart_state = std::make_shared<tonic::DartState>();
        auto listener = fml::MakeRefCounted<PlatformListenerDartPort>(
            tonic::DartConverter<int64_t>::FromDart(
                Dart_GetNativeArgument(args, 0)),
            1);

        uint8_t* data = static_cast<uint8_t*>(malloc(100));
        auto mapping = std::make_unique<fml::MallocMapping>(data, 100);
        listener->Send(101, "test/channel", move(mapping));
      };

  AddNativeCallback("CallPlatformListenerDartPort",
                    CREATE_NATIVE_ENTRY(nativeCallPlatformListenerDartPort));

  auto nativeFinishCallResponse = [message_latch,
                                   &did_pass](Dart_NativeArguments args) {
    did_pass =
        tonic::DartConverter<bool>::FromDart(Dart_GetNativeArgument(args, 0));
    message_latch->Signal();
  };

  AddNativeCallback("FinishCallResponse",
                    CREATE_NATIVE_ENTRY(nativeFinishCallResponse));

  Settings settings = CreateSettingsForFixture();

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("platformListenerDartPortTest");

  shell->RunEngine(std::move(configuration), [](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();

  ASSERT_TRUE(did_pass);
  DestroyShell(std::move(shell), task_runners);
}

}  // namespace testing
}  // namespace flutter
