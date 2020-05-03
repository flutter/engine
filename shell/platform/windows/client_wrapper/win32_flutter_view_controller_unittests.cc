// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "flutter/shell/platform/windows/client_wrapper/include/flutter/win32_flutter_view_controller.h"
#include "flutter/shell/platform/windows/client_wrapper/testing/stub_flutter_windows_api.h"
#include "gtest/gtest.h"

namespace flutter {

namespace {

// Stub implementation to validate calls to the API.
class TestWindowsApi : public testing::StubFlutterWindowsApi {
  FlutterDesktopViewControllerRef V2CreateViewControllerWindow(
      int width,
      int height,
      const FlutterDesktopEngineProperties& engine_properties,
      HWND windowrendertarget,
      HostEnvironmentState hostwindow
      ) override {
    return reinterpret_cast<FlutterDesktopViewControllerRef>(1);
  }
};

}  // namespace

TEST(Win32FlutterViewControllerTest, CreateDestroy) {
  DartProject project(L"data");
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api = static_cast<TestWindowsApi*>(scoped_api_stub.stub());
  { Win32FlutterViewController controller(100, 100, project, static_cast<void*>(nullptr)); }
}

TEST(Win32FlutterViewControllerTest, GetView) {
  DartProject project(L"data");
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api = static_cast<TestWindowsApi*>(scoped_api_stub.stub());
  Win32FlutterViewController controller(100, 100, project, static_cast<HWND>(nullptr));
  EXPECT_NE(controller.view(), nullptr);
}

}  // namespace flutter
