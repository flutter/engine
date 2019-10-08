// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/texture_registrar.h"
#include "flutter/shell/platform/common/cpp/client_wrapper/testing/stub_flutter_api.h"
#include "gtest/gtest.h"

namespace flutter {

namespace {

// Stub implementation to validate calls to the API.
class TestApi : public testing::StubFlutterApi {
 public:
  // |flutter::testing::StubFlutterApi|
  bool MessengerSend(const char* channel,
                     const uint8_t* message,
                     const size_t message_size) override {
    last_data_sent_ = message;
    return message_engine_result;
  }
  bool MessengerSendWithReply(const char* channel,
                              const uint8_t* message,
                              const size_t message_size,
                              const FlutterDesktopBinaryReply reply,
                              void* user_data) override {
    last_data_sent_ = message;
    return message_engine_result;
  }

  const uint8_t* last_data_sent() { return last_data_sent_; }

  int64_t RegisterExternalTexture(
    FlutterTexutreCallback texture_callback,
    void* user_data) override {
      last_texture_id_ = 1;
      return last_texture_id_;
  }

  bool UnregisterExternalTexture(int64_t texture_id) override {
    if(texture_id == 2) {
      last_texture_id_ = -2;
      return true;
    }
    return false;
  }

  bool TextureFrameAvailable(int64_t texture_id) override {
     if(texture_id == 1) {
      last_texture_id_ = 2;
      return true;
    }
    return false;
  }

  int64_t last_texture_id() { return last_texture_id_; }

 private:
  const uint8_t* last_data_sent_ = nullptr;
  int64_t last_texture_id_ = -1;
};

}  // namespace

// Tests that the registrar returns a messenger that calls through to the C API.
TEST(MethodCallTest, MessengerSend) {
  testing::ScopedStubFlutterApi scoped_api_stub(std::make_unique<TestApi>());
  auto test_api = static_cast<TestApi*>(scoped_api_stub.stub());

  auto dummy_registrar_handle =
      reinterpret_cast<FlutterDesktopPluginRegistrarRef>(1);
  PluginRegistrar registrar(dummy_registrar_handle);
  BinaryMessenger* messenger = registrar.messenger();

  std::vector<uint8_t> message = {1, 2, 3, 4};
  messenger->Send("some_channel", &message[0], message.size());
  EXPECT_EQ(test_api->last_data_sent(), &message[0]);
}

// Tests texture register that calls through to the C API.
TEST(MethodCallTest, RegisterTexture) {
  testing::ScopedStubFlutterApi scoped_api_stub(std::make_unique<TestApi>());
  auto test_api = static_cast<TestApi*>(scoped_api_stub.stub());

  auto dummy_registrar_handle =
      reinterpret_cast<FlutterDesktopPluginRegistrarRef>(1);
  PluginRegistrar registrar(dummy_registrar_handle);
  TextureRegistrar* textures = registrar.textures();

  EXPECT_EQ(test_api->last_texture_id(), -1);
  int64_t texture_id = textures->RegisterTexture(nullptr);
  EXPECT_EQ(texture_id, 1);
  textures->MarkTextureFrameAvailable(texture_id);
  EXPECT_EQ(test_api->last_texture_id(), 2);
  textures->UnregisterTexture(2);
  EXPECT_EQ(test_api->last_texture_id(), -2);
}

}  // namespace flutter
