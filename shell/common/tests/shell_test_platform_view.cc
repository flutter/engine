// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/tests/shell_test_platform_view.h"

namespace flutter {
namespace testing {

ShellTestPlatformView::ShellTestPlatformView(
    PlatformView::Delegate& delegate,
    TaskRunners task_runners,
    std::shared_ptr<ShellTestVsyncClock> vsync_clock,
    CreateVsyncWaiter create_vsync_waiter,
    ShellTestSurface::ClientRenderingAPI api)
    : PlatformView(delegate, std::move(task_runners)),
      vsync_clock_(std::move(vsync_clock)),
      create_vsync_waiter_(std::move(create_vsync_waiter)),
      shell_surface_(ShellTestSurface::CreateTestSurface(api)) {
  FML_CHECK(vsync_clock_);
  FML_CHECK(create_vsync_waiter_);
}

ShellTestPlatformView::~ShellTestPlatformView() = default;

std::unique_ptr<VsyncWaiter> ShellTestPlatformView::CreateVSyncWaiter() {
  return create_vsync_waiter_();
}

void ShellTestPlatformView::SimulateVSync() {
  vsync_clock_->SimulateVSync();
}

// |PlatformView|
std::unique_ptr<Surface> ShellTestPlatformView::CreateRenderingSurface() {
  if (auto surface = shell_surface_->CreateRenderingSurface()) {
    return surface;
  }
  FML_CHECK(false) << "Could not obtain the rendering surface.";
  return nullptr;
}

// |PlatformView|
PointerDataDispatcherMaker ShellTestPlatformView::GetDispatcherMaker() {
  return [](DefaultPointerDataDispatcher::Delegate& delegate) {
    return std::make_unique<SmoothPointerDataDispatcher>(delegate);
  };
}

}  // namespace testing
}  // namespace flutter
