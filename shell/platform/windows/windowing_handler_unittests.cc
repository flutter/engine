// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "flutter/shell/platform/windows/windowing_handler.h"

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_result_functions.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"
#include "flutter/shell/platform/windows/flutter_host_window_controller.h"
#include "flutter/shell/platform/windows/testing/flutter_windows_engine_builder.h"
#include "flutter/shell/platform/windows/testing/test_binary_messenger.h"
#include "flutter/shell/platform/windows/testing/windows_test.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

namespace {
using ::testing::_;
using ::testing::Eq;
using ::testing::NiceMock;
using ::testing::Optional;
using ::testing::Return;
using ::testing::StrEq;

constexpr char kChannelName[] = "flutter/windowing";

constexpr char kCreateWindowMethod[] = "createWindow";
constexpr char kCreateDialogMethod[] = "createDialog";
constexpr char kCreateSatelliteMethod[] = "createSatellite";
constexpr char kCreatePopupMethod[] = "createPopup";
constexpr char kDestroyWindowMethod[] = "destroyWindow";

constexpr char kAnchorRectKey[] = "anchorRect";
constexpr char kParentKey[] = "parent";
constexpr char kPositionerChildAnchorKey[] = "positionerChildAnchor";
constexpr char kPositionerConstraintAdjustmentKey[] =
    "positionerConstraintAdjustment";
constexpr char kPositionerOffsetKey[] = "positionerOffset";
constexpr char kPositionerParentAnchorKey[] = "positionerParentAnchor";
constexpr char kSizeKey[] = "size";
constexpr char kViewIdKey[] = "viewId";

void SimulateWindowingMessage(TestBinaryMessenger* messenger,
                              const std::string& method_name,
                              std::unique_ptr<EncodableValue> arguments,
                              MethodResult<EncodableValue>* result_handler) {
  MethodCall<> call(method_name, std::move(arguments));

  auto message = StandardMethodCodec::GetInstance().EncodeMethodCall(call);

  EXPECT_TRUE(messenger->SimulateEngineMessage(
      kChannelName, message->data(), message->size(),
      [&result_handler](const uint8_t* reply, size_t reply_size) {
        StandardMethodCodec::GetInstance().DecodeAndProcessResponseEnvelope(
            reply, reply_size, result_handler);
      }));
}

class MockFlutterHostWindowController : public FlutterHostWindowController {
 public:
  MockFlutterHostWindowController(FlutterWindowsEngine* engine)
      : FlutterHostWindowController(engine) {}
  ~MockFlutterHostWindowController() = default;

  MOCK_METHOD(std::optional<WindowMetadata>,
              CreateHostWindow,
              (std::wstring const& title,
               WindowSize const& size,
               WindowArchetype archetype,
               std::optional<WindowPositioner> positioner,
               std::optional<FlutterViewId> parent_view_id),
              (override));
  MOCK_METHOD(bool, DestroyHostWindow, (FlutterViewId view_id), (override));

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(MockFlutterHostWindowController);
};

bool operator==(WindowPositioner const& lhs, WindowPositioner const& rhs) {
  return lhs.anchor_rect == rhs.anchor_rect &&
         lhs.parent_anchor == rhs.parent_anchor &&
         lhs.child_anchor == rhs.child_anchor && lhs.offset == rhs.offset &&
         lhs.constraint_adjustment == rhs.constraint_adjustment;
}

MATCHER_P(WindowPositionerEq, expected, "WindowPositioner matches expected") {
  return arg.anchor_rect == expected.anchor_rect &&
         arg.parent_anchor == expected.parent_anchor &&
         arg.child_anchor == expected.child_anchor &&
         arg.offset == expected.offset &&
         arg.constraint_adjustment == expected.constraint_adjustment;
}
}  // namespace

class WindowingHandlerTest : public WindowsTest {
 public:
  WindowingHandlerTest() = default;
  virtual ~WindowingHandlerTest() = default;

 protected:
  void SetUp() override {
    FlutterWindowsEngineBuilder builder(GetContext());
    engine_ = builder.Build();

    mock_controller_ =
        std::make_unique<NiceMock<MockFlutterHostWindowController>>(
            engine_.get());

    ON_CALL(*mock_controller_, CreateHostWindow)
        .WillByDefault(Return(WindowMetadata{}));
    ON_CALL(*mock_controller_, DestroyHostWindow).WillByDefault(Return(true));
  }

  MockFlutterHostWindowController* controller() {
    return mock_controller_.get();
  }

 private:
  std::unique_ptr<FlutterWindowsEngine> engine_;
  std::unique_ptr<NiceMock<MockFlutterHostWindowController>> mock_controller_;

  FML_DISALLOW_COPY_AND_ASSIGN(WindowingHandlerTest);
};

TEST_F(WindowingHandlerTest, HandleCreateRegularWindow) {
  TestBinaryMessenger messenger;
  WindowingHandler windowing_handler(&messenger, controller());

  WindowSize const size = {800, 600};
  EncodableMap const arguments = {
      {EncodableValue(kSizeKey),
       EncodableValue(EncodableList{EncodableValue(size.width),
                                    EncodableValue(size.height)})},
  };

  bool success = false;
  MethodResultFunctions<> result_handler(
      [&success](const EncodableValue* result) { success = true; }, nullptr,
      nullptr);

  EXPECT_CALL(
      *controller(),
      CreateHostWindow(StrEq(L"regular"), size, WindowArchetype::regular,
                       Eq(std::nullopt), Eq(std::nullopt)))
      .Times(1);

  SimulateWindowingMessage(&messenger, kCreateWindowMethod,
                           std::make_unique<EncodableValue>(arguments),
                           &result_handler);

  EXPECT_TRUE(success);
}

TEST_F(WindowingHandlerTest, HandleCreatePopup) {
  TestBinaryMessenger messenger;
  WindowingHandler windowing_handler(&messenger, controller());

  WindowSize const size = {200, 200};
  std::optional<FlutterViewId> const parent_view_id = 0;
  WindowPositioner const positioner = WindowPositioner{
      .anchor_rect = std::optional<WindowRectangle>(
          {.top_left = {0, 0}, .size = {size.width, size.height}}),
      .parent_anchor = WindowPositioner::Anchor::center,
      .child_anchor = WindowPositioner::Anchor::center,
      .offset = {0, 0},
      .constraint_adjustment = WindowPositioner::ConstraintAdjustment::none};
  EncodableMap const arguments = {
      {EncodableValue(kSizeKey),
       EncodableValue(EncodableList{EncodableValue(size.width),
                                    EncodableValue(size.height)})},
      {EncodableValue(kAnchorRectKey),
       EncodableValue(
           EncodableList{EncodableValue(positioner.anchor_rect->top_left.x),
                         EncodableValue(positioner.anchor_rect->top_left.y),
                         EncodableValue(positioner.anchor_rect->size.width),
                         EncodableValue(positioner.anchor_rect->size.height)})},
      {EncodableValue(kPositionerParentAnchorKey),
       EncodableValue(static_cast<int>(positioner.parent_anchor))},
      {EncodableValue(kPositionerChildAnchorKey),
       EncodableValue(static_cast<int>(positioner.child_anchor))},
      {EncodableValue(kPositionerOffsetKey),
       EncodableValue(EncodableList{EncodableValue(positioner.offset.x),
                                    EncodableValue(positioner.offset.y)})},
      {EncodableValue(kPositionerConstraintAdjustmentKey),
       EncodableValue(static_cast<int>(positioner.constraint_adjustment))},
      {EncodableValue(kParentKey),
       EncodableValue(static_cast<int>(parent_view_id.value()))}};

  bool success = false;
  MethodResultFunctions<> result_handler(
      [&success](const EncodableValue* result) { success = true; }, nullptr,
      nullptr);

  EXPECT_CALL(*controller(),
              CreateHostWindow(StrEq(L"popup"), size, WindowArchetype::popup,
                               Optional(WindowPositionerEq(positioner)),
                               parent_view_id))
      .Times(1);

  SimulateWindowingMessage(&messenger, kCreatePopupMethod,
                           std::make_unique<EncodableValue>(arguments),
                           &result_handler);

  EXPECT_TRUE(success);
}

TEST_F(WindowingHandlerTest, HandleCreateModalDialog) {
  TestBinaryMessenger messenger;
  WindowingHandler windowing_handler(&messenger, controller());

  WindowSize const size = {400, 300};
  std::optional<FlutterViewId> const parent_view_id = 0;
  EncodableMap const arguments = {
      {EncodableValue(kSizeKey),
       EncodableValue(EncodableList{EncodableValue(size.width),
                                    EncodableValue(size.height)})},
      {EncodableValue(kParentKey),
       EncodableValue(static_cast<int>(parent_view_id.value()))}};

  bool success = false;
  MethodResultFunctions<> result_handler(
      [&success](const EncodableValue* result) { success = true; }, nullptr,
      nullptr);

  EXPECT_CALL(*controller(),
              CreateHostWindow(StrEq(L"dialog"), size, WindowArchetype::dialog,
                               Eq(std::nullopt), parent_view_id))
      .Times(1);

  SimulateWindowingMessage(&messenger, kCreateDialogMethod,
                           std::make_unique<EncodableValue>(arguments),
                           &result_handler);

  EXPECT_TRUE(success);
}

TEST_F(WindowingHandlerTest, HandleCreateModelessDialog) {
  TestBinaryMessenger messenger;
  WindowingHandler windowing_handler(&messenger, controller());

  WindowSize const size = {400, 300};
  EncodableMap const arguments = {
      {EncodableValue(kSizeKey),
       EncodableValue(EncodableList{EncodableValue(size.width),
                                    EncodableValue(size.height)})},
      {EncodableValue(kParentKey), EncodableValue()}};

  bool success = false;
  MethodResultFunctions<> result_handler(
      [&success](const EncodableValue* result) { success = true; }, nullptr,
      nullptr);

  EXPECT_CALL(*controller(),
              CreateHostWindow(StrEq(L"dialog"), size, WindowArchetype::dialog,
                               Eq(std::nullopt), Eq(std::nullopt)))
      .Times(1);

  SimulateWindowingMessage(&messenger, kCreateDialogMethod,
                           std::make_unique<EncodableValue>(arguments),
                           &result_handler);

  EXPECT_TRUE(success);
}

TEST_F(WindowingHandlerTest, HandleCreateSatellite) {
  TestBinaryMessenger messenger;
  WindowingHandler windowing_handler(&messenger, controller());

  WindowSize const size = {200, 300};
  std::optional<FlutterViewId> const parent_view_id = 0;
  WindowPositioner const positioner = WindowPositioner{
      .anchor_rect = std::optional<WindowRectangle>(
          {.top_left = {0, 0}, .size = {size.width, size.height}}),
      .parent_anchor = WindowPositioner::Anchor::center,
      .child_anchor = WindowPositioner::Anchor::center,
      .offset = {0, 0},
      .constraint_adjustment = WindowPositioner::ConstraintAdjustment::none};
  EncodableMap const arguments = {
      {EncodableValue(kSizeKey),
       EncodableValue(EncodableList{EncodableValue(size.width),
                                    EncodableValue(size.height)})},
      {EncodableValue(kAnchorRectKey),
       EncodableValue(
           EncodableList{EncodableValue(positioner.anchor_rect->top_left.x),
                         EncodableValue(positioner.anchor_rect->top_left.y),
                         EncodableValue(positioner.anchor_rect->size.width),
                         EncodableValue(positioner.anchor_rect->size.height)})},
      {EncodableValue(kPositionerParentAnchorKey),
       EncodableValue(static_cast<int>(positioner.parent_anchor))},
      {EncodableValue(kPositionerChildAnchorKey),
       EncodableValue(static_cast<int>(positioner.child_anchor))},
      {EncodableValue(kPositionerOffsetKey),
       EncodableValue(EncodableList{EncodableValue(positioner.offset.x),
                                    EncodableValue(positioner.offset.y)})},
      {EncodableValue(kPositionerConstraintAdjustmentKey),
       EncodableValue(static_cast<int>(positioner.constraint_adjustment))},
      {EncodableValue(kParentKey),
       EncodableValue(static_cast<int>(parent_view_id.value()))}};

  bool success = false;
  MethodResultFunctions<> result_handler(
      [&success](const EncodableValue* result) { success = true; }, nullptr,
      nullptr);

  EXPECT_CALL(*controller(),
              CreateHostWindow(
                  StrEq(L"satellite"), size, WindowArchetype::satellite,
                  Optional(WindowPositionerEq(positioner)), parent_view_id))
      .Times(1);

  SimulateWindowingMessage(&messenger, kCreateSatelliteMethod,
                           std::make_unique<EncodableValue>(arguments),
                           &result_handler);

  EXPECT_TRUE(success);
}

TEST_F(WindowingHandlerTest, HandleDestroyWindow) {
  TestBinaryMessenger messenger;
  WindowingHandler windowing_handler(&messenger, controller());

  EncodableMap const arguments = {
      {EncodableValue(kViewIdKey), EncodableValue(1)},
  };

  bool success = false;
  MethodResultFunctions<> result_handler(
      [&success](const EncodableValue* result) { success = true; }, nullptr,
      nullptr);

  EXPECT_CALL(*controller(), DestroyHostWindow(1)).Times(1);

  SimulateWindowingMessage(&messenger, kDestroyWindowMethod,
                           std::make_unique<EncodableValue>(arguments),
                           &result_handler);

  EXPECT_TRUE(success);
}

}  // namespace testing
}  // namespace flutter
