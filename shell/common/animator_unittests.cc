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
  ASSERT_FALSE(DartVMRef::IsInstanceRunning());
  Settings settings = CreateSettingsForFixture();
  ThreadHost thread_host(
      "io.flutter.test." + GetCurrentTestName() + ".",
      ThreadHost::Type::GPU | ThreadHost::Type::IO | ThreadHost::Type::UI);
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  TaskRunners task_runners("test",
                           fml::MessageLoop::GetCurrent().GetTaskRunner(),
                           thread_host.gpu_thread->GetTaskRunner(),
                           thread_host.ui_thread->GetTaskRunner(),
                           thread_host.io_thread->GetTaskRunner());
  auto shell = Shell::Create(
      std::move(task_runners), settings,
      [](Shell& shell) {
        return std::make_unique<ShellTestPlatformView>(
            shell, shell.GetTaskRunners(), true);
      },
      [](Shell& shell) {
        return std::make_unique<Rasterizer>(shell, shell.GetTaskRunners());
      });
  ASSERT_TRUE(DartVMRef::IsInstanceRunning());

  // we have created a shell!!

  DestroyShell(std::move(shell), std::move(task_runners));
  ASSERT_FALSE(DartVMRef::IsInstanceRunning());
}

}  // namespace testing
}  // namespace flutter
