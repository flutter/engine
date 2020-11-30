// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/path.h"

#include <memory>

#include "flutter/common/task_runners.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell_test.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"

namespace flutter {
namespace testing {

TEST_F(ShellTest, PathVolatilityTracking) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();

  CanvasPath* path;
  auto native_validate_path = [message_latch,
                               &path](Dart_NativeArguments args) {
    auto handle = Dart_GetNativeArgument(args, 0);
    intptr_t peer = 0;
    Dart_Handle result = Dart_GetNativeInstanceField(
        handle, tonic::DartWrappable::kPeerIndex, &peer);
    ASSERT_FALSE(Dart_IsError(result));
    path = reinterpret_cast<CanvasPath*>(peer);
    ASSERT_TRUE(path);
    ASSERT_TRUE(path->path().isVolatile());
    message_latch->Signal();
  };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("ValidatePath", CREATE_NATIVE_ENTRY(native_validate_path));

  std::unique_ptr<Shell> shell =
      CreateShell(std::move(settings), std::move(task_runners));

  ASSERT_TRUE(shell->IsSetup());
  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("createRetainedPath");
  PlatformViewNotifyCreated(shell.get());

  shell->RunEngine(std::move(configuration), [](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  ASSERT_TRUE(path);
  PumpOneFrame(shell.get());
  ASSERT_TRUE(path->path().isVolatile());
  PumpOneFrame(shell.get());
  ASSERT_FALSE(path->path().isVolatile());

  DestroyShell(std::move(shell), std::move(task_runners));
}

TEST_F(ShellTest, PathVolatilityTrackingCollected) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();

  CanvasPath* path;
  auto native_validate_path = [message_latch,
                               &path](Dart_NativeArguments args) {
    auto handle = Dart_GetNativeArgument(args, 0);
    intptr_t peer = 0;
    Dart_Handle result = Dart_GetNativeInstanceField(
        handle, tonic::DartWrappable::kPeerIndex, &peer);
    ASSERT_FALSE(Dart_IsError(result));
    path = reinterpret_cast<CanvasPath*>(peer);
    ASSERT_TRUE(path);
    ASSERT_TRUE(path->path().isVolatile());
    message_latch->Signal();
  };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("ValidatePath", CREATE_NATIVE_ENTRY(native_validate_path));

  std::unique_ptr<Shell> shell =
      CreateShell(std::move(settings), std::move(task_runners));

  ASSERT_TRUE(shell->IsSetup());
  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("createCollectablePath");
  PlatformViewNotifyCreated(shell.get());

  shell->RunEngine(std::move(configuration), [](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  ASSERT_TRUE(path);
  PumpOneFrame(shell.get());
  ASSERT_TRUE(path->path().isVolatile());
  PumpOneFrame(shell.get());
  // Because the path got GC'd, it was removed from the cache and we're the
  // only one holding it.
  ASSERT_TRUE(path->path().isVolatile());

  DestroyShell(std::move(shell), std::move(task_runners));
}

}  // namespace testing
}  // namespace flutter
