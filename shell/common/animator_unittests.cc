// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include <functional>
#include <future>
#include <memory>

#include "flutter/shell/common/animator.h"
#include "flutter/shell/common/shell_test.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST_F(ShellTest, VSyncTargetTime) {
  int64_t begin_frame;
  fml::AutoResetWaitableEvent laaa;
  auto nativeOnBeginFrame = [&laaa, &begin_frame](Dart_NativeArguments args) {
    Dart_Handle exception = nullptr;
    begin_frame =
        tonic::DartConverter<int64_t>::FromArguments(args, 0, exception);
    std::cout << begin_frame << std::endl;
    laaa.Signal();
  };
  AddNativeCallback("NativeOnBeginFrame",
                    CREATE_NATIVE_ENTRY(nativeOnBeginFrame));

  ASSERT_FALSE(DartVMRef::IsInstanceRunning());
  auto settings = CreateSettingsForFixture();

  std::unique_ptr<Shell> shell;

  fml::AutoResetWaitableEvent shell_creation;

  auto platform_task = std::async(std::launch::async, [&]() {
    shell = CreateShell(settings, true);
    ASSERT_TRUE(DartVMRef::IsInstanceRunning());

    auto configuration = RunConfiguration::InferFromSettings(settings);
    ASSERT_TRUE(configuration.IsValid());
    configuration.SetEntrypoint("onBeginFrameMain");

    RunEngine(shell.get(), std::move(configuration));
    shell_creation.Signal();
  });

  shell_creation.Wait();
  FML_LOG(ERROR) << "Shell Created!!!";

  fml::TaskRunner::RunNowOrPostTask(shell->GetTaskRunners().GetUITaskRunner(),
                                    [engine = shell->GetEngine()]() {
                                      if (engine) {
                                        engine->ScheduleFrame(false);
                                      }
                                    });

  FML_LOG(ERROR) << "HEADAHHDEDA 000";

  fml::TaskRunner::RunNowOrPostTask(shell->GetTaskRunners().GetIOTaskRunner(),
                                    [&]() {
                                      // ShellTestPlatformView*
                                      // test_platform_view =
                                      //     static_cast<ShellTestPlatformView*>(shell->GetPlatformView().get());
                                      FML_LOG(ERROR) << "asda 1";
                                      // test_platform_view->SimulateVSync();
                                      FML_LOG(ERROR) << "asda 2";
                                      bool ig = false;
                                      ShellTest::VSyncFlush(shell.get(), ig);
                                      FML_LOG(ERROR) << "asda 4";
                                    });

  FML_LOG(ERROR) << "HEADAHHDEDA 111";

  laaa.Wait();

  TaskRunners task_runners = GetTaskRunnersForFixture();
  DestroyShell(std::move(shell), std::move(task_runners));
  ASSERT_FALSE(DartVMRef::IsInstanceRunning());
}

}  // namespace testing
}  // namespace flutter
