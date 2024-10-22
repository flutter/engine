// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/client_wrapper/include/flutter/encodable_value.h"
#include "flutter/shell/platform/windows/client_wrapper/include/flutter/flutter_win32_window.h"
#include "flutter/shell/platform/windows/client_wrapper/include/flutter/flutter_window_controller.h"
#include "flutter/shell/platform/windows/client_wrapper/testing/stub_flutter_windows_api.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsNull;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrEq;

namespace flutter {

namespace {

HWND const k_hwnd{reinterpret_cast<HWND>(-1)};

// Stub implementation to validate calls to the API.
class TestWindowsApi : public testing::StubFlutterWindowsApi {
 public:
  // |flutter::testing::StubFlutterWindowsApi|
  FlutterDesktopViewControllerRef EngineCreateViewController(
      const FlutterDesktopViewControllerProperties* properties) override {
    return reinterpret_cast<FlutterDesktopViewControllerRef>(2);
  }
};

// Mocked classes
class MockWin32Wrapper : public Win32Wrapper {
 public:
  MOCK_METHOD(HWND,
              CreateWindowEx,
              (DWORD dwExStyle,
               LPCWSTR lpClassName,
               LPCWSTR lpWindowName,
               DWORD dwStyle,
               int X,
               int Y,
               int nWidth,
               int nHeight,
               HWND hWndParent,
               HMENU hMenu,
               HINSTANCE hInstance,
               LPVOID lpParam),
              (override));
  MOCK_METHOD(BOOL, DestroyWindow, (HWND hWnd), (override));
};

class MockMethodResult : public MethodResult<> {
 public:
  MOCK_METHOD(void,
              SuccessInternal,
              (EncodableValue const* result),
              (override));
  MOCK_METHOD(void,
              ErrorInternal,
              (std::string const& error_code,
               std::string const& error_message,
               EncodableValue const* error_details),
              (override));
  MOCK_METHOD(void, NotImplementedInternal, (), (override));
};

class MockFlutterWindowController : public FlutterWindowController {
 public:
  using FlutterWindowController::MessageHandler;
  using FlutterWindowController::MethodCallHandler;

  MockFlutterWindowController(std::shared_ptr<Win32Wrapper> wrapper)
      : FlutterWindowController(std::move(wrapper)) {}

  MOCK_METHOD(void,
              SendOnWindowCreated,
              (FlutterViewId view_id,
               std::optional<FlutterViewId> parent_view_id),
              (override, const));
  MOCK_METHOD(void,
              SendOnWindowDestroyed,
              (FlutterViewId view_id),
              (override, const));
  MOCK_METHOD(void,
              SendOnWindowChanged,
              (FlutterViewId view_id),
              (override, const));
};

// Test fixture
class FlutterWindowControllerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    DartProject project(L"test");
    engine_ = std::make_shared<FlutterEngine>(project);
    mock_win32_ = std::make_shared<NiceMock<MockWin32Wrapper>>();
    mock_controller_ =
        std::make_unique<NiceMock<MockFlutterWindowController>>(mock_win32_);
    mock_controller_->SetEngine(engine_);

    ON_CALL(*mock_win32_, CreateWindowEx).WillByDefault(Return(k_hwnd));
    ON_CALL(*mock_win32_, DestroyWindow).WillByDefault([&](HWND hwnd) {
      mock_controller_->MessageHandler(hwnd, WM_NCDESTROY, 0, 0);
      return TRUE;
    });
  }

  std::shared_ptr<FlutterEngine> engine_;
  std::shared_ptr<NiceMock<MockWin32Wrapper>> mock_win32_;
  std::unique_ptr<NiceMock<MockFlutterWindowController>> mock_controller_;
};

}  // namespace

TEST_F(FlutterWindowControllerTest, CreateRegularWindow) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  auto const title{L"window"};
  WindowSize const size{800, 600};
  auto const archetype{WindowArchetype::regular};
  std::optional<WindowPositioner> const positioner;
  std::optional<FlutterViewId> const parent_view_id;

  EXPECT_CALL(*mock_win32_,
              CreateWindowEx(0, _, StrEq(title), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, Gt(size.width),
                             Gt(size.height), IsNull(), _, _, _))
      .Times(1);

  auto const result{mock_controller_->CreateFlutterWindow(
      title, size, archetype, positioner, parent_view_id)};

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->view_id, 1);
  EXPECT_FALSE(result->parent_id.has_value());
  EXPECT_EQ(result->archetype, archetype);
}

TEST_F(FlutterWindowControllerTest, DestroyWindow) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  auto const title{L"window"};
  WindowSize const size{800, 600};
  auto const archetype{WindowArchetype::regular};

  auto const create_result{mock_controller_->CreateFlutterWindow(
      title, size, archetype, std::nullopt, std::nullopt)};

  ASSERT_TRUE(create_result.has_value());

  EXPECT_CALL(*mock_win32_, DestroyWindow(k_hwnd)).Times(1);

  EXPECT_TRUE(mock_controller_->DestroyFlutterWindow(1));
}

TEST_F(FlutterWindowControllerTest, DestroyWindowWithInvalidView) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  auto const title{L"window"};
  WindowSize const size{800, 600};
  auto const archetype{WindowArchetype::regular};

  auto const create_result{mock_controller_->CreateFlutterWindow(
      title, size, archetype, std::nullopt, std::nullopt)};

  ASSERT_TRUE(create_result.has_value());

  EXPECT_CALL(*mock_win32_, DestroyWindow(k_hwnd)).Times(0);

  EXPECT_FALSE(mock_controller_->DestroyFlutterWindow(9999));

  Mock::VerifyAndClearExpectations(mock_win32_.get());
}

TEST_F(FlutterWindowControllerTest, SendOnWindowCreated) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  auto const title{L"window"};
  WindowSize const size{800, 600};
  auto const archetype{WindowArchetype::regular};

  EXPECT_CALL(*mock_controller_, SendOnWindowCreated(1, Eq(std::nullopt)))
      .Times(1);

  auto const create_result{mock_controller_->CreateFlutterWindow(
      title, size, archetype, std::nullopt, std::nullopt)};
}

TEST_F(FlutterWindowControllerTest, SendOnWindowDestroyed) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  auto const title{L"window"};
  WindowSize const size{800, 600};
  auto const archetype{WindowArchetype::regular};

  auto const create_result{mock_controller_->CreateFlutterWindow(
      title, size, archetype, std::nullopt, std::nullopt)};

  ASSERT_TRUE(create_result.has_value());

  EXPECT_CALL(*mock_controller_, SendOnWindowDestroyed).Times(1);

  mock_controller_->DestroyFlutterWindow(1);
}

TEST_F(FlutterWindowControllerTest, SendOnWindowChangedWhenWindowIsResized) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  auto const title{L"window"};
  WindowSize const size{800, 600};
  auto const archetype{WindowArchetype::regular};

  auto const create_result{mock_controller_->CreateFlutterWindow(
      title, size, archetype, std::nullopt, std::nullopt)};

  EXPECT_CALL(*mock_controller_, SendOnWindowChanged(1)).Times(1);

  mock_controller_->MessageHandler(k_hwnd, WM_SIZE, 0, 0);
}

TEST_F(FlutterWindowControllerTest, CreateRegularWindowUsingMethodCall) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  WindowSize const size{800, 600};

  EncodableMap const arguments{
      {EncodableValue("size"),
       EncodableValue(EncodableList{EncodableValue(size.width),
                                    EncodableValue(size.height)})},
  };
  MethodCall<> call("createWindow",
                    std::make_unique<EncodableValue>(arguments));

  NiceMock<MockMethodResult> mock_result;

  EXPECT_CALL(mock_result, SuccessInternal(_)).Times(1);
  EXPECT_CALL(*mock_win32_,
              CreateWindowEx(0, _, StrEq(L"regular"), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, Gt(size.width),
                             Gt(size.height), IsNull(), _, _, _))
      .Times(1);

  mock_controller_->MethodCallHandler(call, mock_result);
}

TEST_F(FlutterWindowControllerTest, DestroyWindowUsingMethodCall) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>());
  auto test_api{static_cast<TestWindowsApi*>(scoped_api_stub.stub())};

  auto const title{L"window"};
  WindowSize size{800, 600};
  auto const archetype{WindowArchetype::regular};
  auto create_result{mock_controller_->CreateFlutterWindow(
      title, size, archetype, std::nullopt, std::nullopt)};

  ASSERT_TRUE(create_result.has_value());

  EncodableMap const arguments{
      {EncodableValue("viewId"),
       EncodableValue(static_cast<int>(create_result->view_id))},
  };
  MethodCall<> call("destroyWindow",
                    std::make_unique<EncodableValue>(arguments));

  NiceMock<MockMethodResult> mock_result;

  EXPECT_CALL(mock_result, SuccessInternal(_)).Times(1);
  EXPECT_CALL(*mock_win32_, DestroyWindow(k_hwnd)).Times(1);

  mock_controller_->MethodCallHandler(call, mock_result);
}

}  // namespace flutter
