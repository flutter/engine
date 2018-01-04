// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/testing/test_runner.h"

#include <iostream>

#include "flutter/fml/message_loop.h"
#include "flutter/shell/common/null_rasterizer.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/common/shell.h"

namespace shell {

TestRunner::TestRunner() {
  // We'll use the current thread as the platform thread.
  thread_host_ = {"io.flutter.test.", ThreadHost::Type::GPU |
                                          ThreadHost::Type::UI |
                                          ThreadHost::Type::IO};

  fml::MessageLoop::EnsureInitializedForCurrentThread();
  blink::TaskRunners task_runners(
      "io.flutter.test", fml::MessageLoop::GetCurrent().GetTaskRunner(),
      thread_host_.gpu_thread->GetTaskRunner(),
      thread_host_.ui_thread->GetTaskRunner(),
      thread_host_.io_thread->GetTaskRunner());

  Shell::CreateCallback<PlatformView> on_create_platform_view =
      [](Shell& shell) {
        return std::make_unique<PlatformView>(shell, shell.GetTaskRunners());
      };

  Shell::CreateCallback<Rasterizer> on_create_rasterizer = [](Shell& shell) {
    return std::make_unique<NullRasterizer>(shell.GetTaskRunners());
  };

  const blink::Settings settings{};

  shell_ = Shell::Create(std::move(task_runners), settings,
                         on_create_platform_view, on_create_rasterizer);
  FXL_CHECK(shell_);

  blink::ViewportMetrics metrics;
  metrics.device_pixel_ratio = 3.0;
  metrics.physical_width = 2400;   // 800 at 3x resolution
  metrics.physical_height = 1800;  // 600 at 3x resolution

  shell_->GetTaskRunners().GetUITaskRunner()->PostTask(
      [ engine = shell_->GetEngine(), metrics ] {
        if (engine) {
          engine->SetViewportMetrics(metrics);
        }
      });
}

TestRunner::~TestRunner() = default;

TestRunner& TestRunner::Shared() {
  static TestRunner* g_test_runner = nullptr;
  if (!g_test_runner)
    g_test_runner = new TestRunner();
  return *g_test_runner;
}

void TestRunner::Run(RunConfiguration config) {
  shell_->GetTaskRunners().GetUITaskRunner()->PostTask(
      [ engine = shell_->GetEngine(), config = std::move(config) ] {
        if (engine) {
          auto success = engine->Run(config);
          if (!success) {
            FXL_LOG(INFO) << "Could not run test with configuration: "
                          << config.ToString();
          }
        }
      });
}

}  // namespace shell
