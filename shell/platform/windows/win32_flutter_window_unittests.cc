// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/cpp/json_message_codec.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "flutter/shell/platform/windows/testing/engine_embedder_api_modifier.h"
#include "flutter/shell/platform/windows/testing/mock_window_binding_handler.h"
#include "flutter/shell/platform/windows/testing/win32_flutter_window_test.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <rapidjson/document.h>

using testing::_;
using testing::Invoke;

namespace flutter {
namespace testing {

namespace {

class SpyKeyEventHandler : public KeyEventHandler {
 public:
  SpyKeyEventHandler(flutter::BinaryMessenger* messenger,
                     SendInputDelegate delegate = SendInput)
      : KeyEventHandler(messenger, delegate) {
    KeyEventHandler* super = reinterpret_cast<KeyEventHandler*>(this);
    ON_CALL(*this, KeyboardHook(_, _, _, _, _, _))
        .WillByDefault(Invoke(super, &KeyEventHandler::KeyboardHook));
    ON_CALL(*this, TextHook(_, _))
        .WillByDefault(Invoke(super, &KeyEventHandler::TextHook));
  }

  MOCK_METHOD6(KeyboardHook,
               bool(FlutterWindowsView* window,
                    int key,
                    int scancode,
                    int action,
                    char32_t character,
                    bool extended));
  MOCK_METHOD2(TextHook,
               void(FlutterWindowsView* window, const std::u16string& text));
};

class SpyTextInputPlugin : public TextInputPlugin {
 public:
  SpyTextInputPlugin(flutter::BinaryMessenger* messenger)
      : TextInputPlugin(messenger) {
    TextInputPlugin* super = reinterpret_cast<TextInputPlugin*>(this);
    ON_CALL(*this, KeyboardHook(_, _, _, _, _, _))
        .WillByDefault(Invoke(super, &TextInputPlugin::KeyboardHook));
    ON_CALL(*this, TextHook(_, _))
        .WillByDefault(Invoke(super, &TextInputPlugin::TextHook));
  }

  MOCK_METHOD6(KeyboardHook,
               bool(FlutterWindowsView* window,
                    int key,
                    int scancode,
                    int action,
                    char32_t character,
                    bool extended));
  MOCK_METHOD2(TextHook,
               void(FlutterWindowsView* window, const std::u16string& text));
};

class TestFlutterWindowsView : public FlutterWindowsView {
 public:
  TestFlutterWindowsView(std::unique_ptr<WindowBindingHandler> window_binding)
      : FlutterWindowsView(std::move(window_binding)) {}

  SpyKeyEventHandler* key_event_handler;
  SpyTextInputPlugin* text_input_plugin;

 protected:
  void RegisterKeyboardHookHandlers(
      flutter::BinaryMessenger* messenger) override {
    auto spy_key_event_handler =
        std::make_unique<SpyKeyEventHandler>(messenger);
    auto spy_text_input_plugin =
        std::make_unique<SpyTextInputPlugin>(messenger);
    key_event_handler = spy_key_event_handler.get();
    text_input_plugin = spy_text_input_plugin.get();
    AddKeyboardHookHandler(std::move(spy_key_event_handler));
    AddKeyboardHookHandler(std::move(spy_text_input_plugin));
  }
};

struct TestResponseHandle {
  FlutterDesktopBinaryReply callback;
  void* user_data;
};

using TestResponseCallback = bool();

// Returns an engine instance configured with dummy project path values.
std::unique_ptr<FlutterWindowsEngine> GetTestEngine(
    TestResponseCallback test_response) {
  FlutterDesktopEngineProperties properties = {};
  properties.assets_path = L"C:\\foo\\flutter_assets";
  properties.icu_data_path = L"C:\\foo\\icudtl.dat";
  properties.aot_library_path = L"C:\\foo\\aot.so";
  FlutterProjectBundle project(properties);
  auto engine = std::make_unique<FlutterWindowsEngine>(project);

  EngineEmbedderApiModifier modifier(engine.get());
  // Force the non-AOT path unless overridden by the test.
  modifier.embedder_api().RunsAOTCompiledDartCode = []() { return false; };

  modifier.embedder_api().PlatformMessageCreateResponseHandle =
      MOCK_ENGINE_PROC(
          PlatformMessageCreateResponseHandle,
          [](auto engine, auto data_callback, auto user_data,
             auto response_out) {
            TestResponseHandle* response_handle = new TestResponseHandle();
            response_handle->user_data = user_data;
            response_handle->callback = data_callback;
            *response_out =
                reinterpret_cast<FlutterPlatformMessageResponseHandle*>(
                    response_handle);
            return kSuccess;
          });

  modifier.embedder_api().SendPlatformMessage = MOCK_ENGINE_PROC(
      SendPlatformMessage,
      ([test_response](FLUTTER_API_SYMBOL(FlutterEngine) engine,
                       const FlutterPlatformMessage* message) {
        rapidjson::Document document;
        auto& allocator = document.GetAllocator();
        document.SetObject();
        document.AddMember("handled", test_response(), allocator);
        auto encoded =
            flutter::JsonMessageCodec::GetInstance().EncodeMessage(document);
        const TestResponseHandle* response_handle =
            reinterpret_cast<const TestResponseHandle*>(
                message->response_handle);
        if (response_handle->callback != nullptr) {
          response_handle->callback(encoded->data(), encoded->size(),
                                    response_handle->user_data);
        }
        return kSuccess;
      }));

  modifier.embedder_api().PlatformMessageReleaseResponseHandle =
      MOCK_ENGINE_PROC(
          PlatformMessageReleaseResponseHandle,
          [](FLUTTER_API_SYMBOL(FlutterEngine) engine,
             FlutterPlatformMessageResponseHandle* response) {
            const TestResponseHandle* response_handle =
                reinterpret_cast<const TestResponseHandle*>(response);
            delete response_handle;
            return kSuccess;
          });

  return engine;
}
}  // namespace

TEST(Win32FlutterWindowTest, CreateDestroy) {
  Win32FlutterWindowTest window(800, 600);
  ASSERT_TRUE(TRUE);
}

TEST(Win32FlutterWindowTest, KeyEventPropagation) {
  Win32FlutterWindowTest window(800, 600);
  auto window_binding_handler = std::make_unique<MockWindowBindingHandler>();
  EXPECT_CALL(*window_binding_handler, SetView(_)).Times(1);
  EXPECT_CALL(*window_binding_handler, GetRenderTarget()).Times(1);
  EXPECT_CALL(*window_binding_handler, GetPhysicalWindowBounds()).Times(1);
  EXPECT_CALL(*window_binding_handler, GetDpiScale()).Times(1);
  TestFlutterWindowsView flutter_windows_view(
      std::move(window_binding_handler));
  window.SetView(&flutter_windows_view);
  flutter_windows_view.SetEngine(
      std::move(GetTestEngine([]() { return true; })));
  EXPECT_CALL(*flutter_windows_view.key_event_handler,
              KeyboardHook(_, 65, 30, WM_KEYDOWN, 65, false))
      .Times(1);
  EXPECT_CALL(*flutter_windows_view.text_input_plugin,
              KeyboardHook(_, 65, 30, WM_KEYDOWN, 65, false))
      .Times(0);
  EXPECT_EQ(flutter_windows_view.OnKey(65, 30, WM_KEYDOWN, 65, false), true);
}

}  // namespace testing
}  // namespace flutter
