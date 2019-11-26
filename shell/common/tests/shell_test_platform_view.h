// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_PLATFORM_VIEW_H_
#define FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_PLATFORM_VIEW_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/common/tests/shell_test_surface.h"
#include "flutter/shell/common/tests/shell_test_vsync_waiter.h"

namespace flutter {
namespace testing {

class ShellTestPlatformView : public PlatformView {
 public:
  ShellTestPlatformView(PlatformView::Delegate& delegate,
                        TaskRunners task_runners,
                        std::shared_ptr<ShellTestVsyncClock> vsync_clock,
                        CreateVsyncWaiter create_vsync_waiter,
                        ShellTestSurface::ClientRenderingAPI api);

  // |PlatformView|
  ~ShellTestPlatformView() override;

  void SimulateVSync();

 private:
  std::shared_ptr<ShellTestVsyncClock> vsync_clock_;
  CreateVsyncWaiter create_vsync_waiter_;
  std::unique_ptr<ShellTestSurface> shell_surface_;

  // |PlatformView|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  // |PlatformView|
  std::unique_ptr<VsyncWaiter> CreateVSyncWaiter() override;

  // |PlatformView|
  PointerDataDispatcherMaker GetDispatcherMaker() override;

  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestPlatformView);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_TESTS_SHELL_TEST_PLATFORM_VIEW_H_
