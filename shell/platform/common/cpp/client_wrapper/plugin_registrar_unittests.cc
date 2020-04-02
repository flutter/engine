// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/plugin_registrar.h"
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

  void MessengerSetCallback(const char* channel,
                            FlutterDesktopMessageCallback callback,
                            void* user_data) override {
    last_message_callback_set_ = callback;
  }

  void RegistrarSetDestructionHandler(
      FlutterDesktopOnRegistrarDestroyed callback) override {
    last_destruction_callback_set_ = callback;
  }

  const uint8_t* last_data_sent() { return last_data_sent_; }
  FlutterDesktopMessageCallback last_message_callback_set() {
    return last_message_callback_set_;
  }
  FlutterDesktopOnRegistrarDestroyed last_destruction_callback_set() {
    return last_destruction_callback_set_;
  }

 private:
  const uint8_t* last_data_sent_ = nullptr;
  FlutterDesktopMessageCallback last_message_callback_set_ = nullptr;
  FlutterDesktopOnRegistrarDestroyed last_destruction_callback_set_ = nullptr;
};

}  // namespace

// Tests that the registrar returns a messenger that passes Send through to the
// C API.
TEST(PluginRegistrarTest, MessengerSend) {
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

// Tests that the registrar returns a messenger that passes callback
// registration and unregistration through to the C API.
TEST(PluginRegistrarTest, MessengerSetMessageHandler) {
  testing::ScopedStubFlutterApi scoped_api_stub(std::make_unique<TestApi>());
  auto test_api = static_cast<TestApi*>(scoped_api_stub.stub());

  auto dummy_registrar_handle =
      reinterpret_cast<FlutterDesktopPluginRegistrarRef>(1);
  PluginRegistrar registrar(dummy_registrar_handle);
  BinaryMessenger* messenger = registrar.messenger();
  const std::string channel_name("foo");

  // Register.
  BinaryMessageHandler binary_handler = [](const uint8_t* message,
                                           const size_t message_size,
                                           BinaryReply reply) {};
  messenger->SetMessageHandler(channel_name, std::move(binary_handler));
  EXPECT_NE(test_api->last_message_callback_set(), nullptr);

  // Unregister.
  messenger->SetMessageHandler(channel_name, nullptr);
  EXPECT_EQ(test_api->last_message_callback_set(), nullptr);
}

// Tests that the registrar manager returns the same instance when getting
// the wrapper for the same reference.
TEST(PluginRegistrarTest, ManagerSameInstance) {
  testing::ScopedStubFlutterApi scoped_api_stub(std::make_unique<TestApi>());
  auto test_api = static_cast<TestApi*>(scoped_api_stub.stub());

  auto dummy_registrar_handle =
      reinterpret_cast<FlutterDesktopPluginRegistrarRef>(1);

  PluginRegistrarManager* manager = PluginRegistrarManager::GetInstance();
  EXPECT_EQ(manager->GetRegistrar<PluginRegistrar>(dummy_registrar_handle),
            manager->GetRegistrar<PluginRegistrar>(dummy_registrar_handle));
}

// Tests that the registrar manager returns different objects for different
// references.
TEST(PluginRegistrarTest, ManagerDifferentInstances) {
  testing::ScopedStubFlutterApi scoped_api_stub(std::make_unique<TestApi>());
  auto test_api = static_cast<TestApi*>(scoped_api_stub.stub());

  auto dummy_registrar_handle_a =
      reinterpret_cast<FlutterDesktopPluginRegistrarRef>(1);
  auto dummy_registrar_handle_b =
      reinterpret_cast<FlutterDesktopPluginRegistrarRef>(2);

  PluginRegistrarManager* manager = PluginRegistrarManager::GetInstance();
  EXPECT_NE(manager->GetRegistrar<PluginRegistrar>(dummy_registrar_handle_a),
            manager->GetRegistrar<PluginRegistrar>(dummy_registrar_handle_b));
}

// Tests that the registrar manager deletes wrappers when the underlying
// reference is destroyed.
TEST(PluginRegistrarTest, ManagerRemovesOnDestruction) {
  testing::ScopedStubFlutterApi scoped_api_stub(std::make_unique<TestApi>());
  auto test_api = static_cast<TestApi*>(scoped_api_stub.stub());

  auto dummy_registrar_handle =
      reinterpret_cast<FlutterDesktopPluginRegistrarRef>(1);
  PluginRegistrarManager* manager = PluginRegistrarManager::GetInstance();
  auto* first_wrapper =
      manager->GetRegistrar<PluginRegistrar>(dummy_registrar_handle);

  // Simulate destruction of the reference.
  EXPECT_NE(test_api->last_destruction_callback_set(), nullptr);
  test_api->last_destruction_callback_set()(dummy_registrar_handle);

  // Requesting the wrapper should now create a new object.
  EXPECT_NE(manager->GetRegistrar<PluginRegistrar>(dummy_registrar_handle),
            first_wrapper);
}

}  // namespace flutter
