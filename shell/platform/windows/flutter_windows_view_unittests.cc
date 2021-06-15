// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <vector>

#include "flutter/shell/platform/common/json_message_codec.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "flutter/shell/platform/windows/flutter_windows_view.h"
#include "flutter/shell/platform/windows/flutter_windows_texture_registrar.h"
#include "flutter/shell/platform/windows/testing/engine_modifier.h"
#include "flutter/shell/platform/windows/testing/mock_window_binding_handler.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

constexpr uint64_t kScanCodeKeyA = 0x1e;

constexpr uint64_t kVirtualKeyA = 0x41;

namespace {

// A struct to use as a FlutterPlatformMessageResponseHandle so it can keep the
// callbacks and user data passed to the engine's
// PlatformMessageCreateResponseHandle for use in the SendPlatformMessage
// overridden function.
struct TestResponseHandle {
  FlutterDesktopBinaryReply callback;
  void* user_data;
};

// The static value to return as the "handled" value from the framework for key
// events. Individual tests set this to change the framework response that the
// test engine simulates.
static bool test_response = false;

static std::vector<int> test_sequences;

// Returns an engine instance configured with dummy project path values, and
// overridden methods for sending platform messages, so that the engine can
// respond as if the framework were connected.
std::unique_ptr<FlutterWindowsEngine> GetTestEngine() {
  FlutterDesktopEngineProperties properties = {};
  properties.assets_path = L"C:\\foo\\flutter_assets";
  properties.icu_data_path = L"C:\\foo\\icudtl.dat";
  properties.aot_library_path = L"C:\\foo\\aot.so";
  FlutterProjectBundle project(properties);
  auto engine = std::make_unique<FlutterWindowsEngine>(project);

  EngineModifier modifier(engine.get());

  modifier.embedder_api().Run = [](
                size_t version, const FlutterRendererConfig* config,
                const FlutterProjectArgs* args, void* user_data,
                FLUTTER_API_SYMBOL(FlutterEngine) * engine_out) {
        *engine_out = reinterpret_cast<FLUTTER_API_SYMBOL(FlutterEngine)>(1);

        return kSuccess;
      };

  modifier.embedder_api().PlatformMessageCreateResponseHandle =
      [](auto engine, auto data_callback, auto user_data, auto response_out) {
        TestResponseHandle* response_handle = new TestResponseHandle();
        response_handle->user_data = user_data;
        response_handle->callback = data_callback;
        *response_out = reinterpret_cast<FlutterPlatformMessageResponseHandle*>(
            response_handle);
        return kSuccess;
      };

  modifier.embedder_api().SendPlatformMessage =
      [](FLUTTER_API_SYMBOL(FlutterEngine) engine,
         const FlutterPlatformMessage* message) {
        if (std::string(message->channel) == std::string("flutter/settings")) {
          return kSuccess;
        }
        rapidjson::Document document;
        test_sequences.push_back(1);
        auto& allocator = document.GetAllocator();
        document.SetObject();
        document.AddMember("handled", test_response, allocator);
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
      };

  modifier.embedder_api().PlatformMessageReleaseResponseHandle =
      [](FLUTTER_API_SYMBOL(FlutterEngine) engine,
         FlutterPlatformMessageResponseHandle* response) {
        const TestResponseHandle* response_handle =
            reinterpret_cast<const TestResponseHandle*>(response);
        delete response_handle;
        return kSuccess;
      };

  modifier.embedder_api().UpdateLocales = [](auto engine, const FlutterLocale** locales,
                                size_t locales_count) {
        return kSuccess;
      };

  modifier.embedder_api().SendWindowMetricsEvent = [](auto engine, const FlutterWindowMetricsEvent* event) {
        return kSuccess;
  };

  modifier.embedder_api().Shutdown = [](auto engine) { return kSuccess; };

  modifier.embedder_api().SendKeyEvent =
      [](FLUTTER_API_SYMBOL(FlutterEngine) engine,
         const FlutterKeyEvent* event,
         FlutterKeyEventCallback callback,
         void* user_data) {
        printf("SendKeyEvent\n");
        test_sequences.push_back(2);
        if (callback != nullptr) {
          callback(test_response, user_data);
        }
        return kSuccess;
      };


  return engine;
}

}  // namespace

TEST(FlutterWindowsViewTest, KeySequence) {
  std::unique_ptr<FlutterWindowsEngine> engine = GetTestEngine();
  engine->RunWithEntrypoint(nullptr);

  auto window_binding_handler =
      std::make_unique<::testing::NiceMock<MockWindowBindingHandler>>();
  FlutterWindowsView view(std::move(window_binding_handler));
  view.SetEngine(std::move(engine));

  view.OnKey(kVirtualKeyA, kScanCodeKeyA, WM_KEYDOWN, 'a', false, false);

  EXPECT_EQ(test_sequences.size(), 2);
  EXPECT_EQ(test_sequences[0], 2);
  EXPECT_EQ(test_sequences[1], 1);
}

}  // namespace testing
}  // namespace flutter
