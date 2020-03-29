// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/event_channel.h"

#include <memory>
#include <string>

#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/cpp/client_wrapper/include/flutter/standard_method_codec.h"
#include "gtest/gtest.h"

namespace flutter {

namespace {

class TestBinaryMessenger : public BinaryMessenger {
 public:
  void Send(const std::string& channel,
            const uint8_t* message,
            const size_t message_size) const override {}

  void Send(const std::string& channel,
            const uint8_t* message,
            const size_t message_size,
            BinaryReply reply) const override {}

  void SetMessageHandler(const std::string& channel,
                         BinaryMessageHandler handler) override {
    last_message_handler_channel_ = channel;
    last_message_handler_ = handler;
  }

  std::string last_message_handler_channel() {
    return last_message_handler_channel_;
  }

  BinaryMessageHandler last_message_handler() { return last_message_handler_; }

 private:
  std::string last_message_handler_channel_;
  BinaryMessageHandler last_message_handler_;
};

}  // namespace

// Tests that SetStreamHandler sets a handler that correctly interacts with
// the binary messenger.
TEST(EventChannelTest, Registration) {
  TestBinaryMessenger messenger;
  const std::string channel_name("some_channel");
  const StandardMethodCodec& codec = StandardMethodCodec::GetInstance();
  EventChannel channel(&messenger, channel_name, &codec);

  bool on_listen_called = false;
  auto onListen = [&on_listen_called](
                      const flutter::EncodableValue* arguments,
                      EventSink<flutter::EncodableValue>* events) {
    on_listen_called = true;
  };
  auto onCancel = [](const flutter::EncodableValue* arguments) {};

  flutter::StreamHandler<flutter::EncodableValue> handler(onListen, onCancel);
  channel.SetStreamHandler(&handler);
  EXPECT_EQ(messenger.last_message_handler_channel(), channel_name);
  EXPECT_NE(messenger.last_message_handler(), nullptr);

  // Send dummy listen message
  MethodCall<flutter::EncodableValue> call("listen", nullptr);
  auto message = codec.EncodeMethodCall(call);
  messenger.last_message_handler()(
      message->data(), message->size(),
      [](const uint8_t* reply, const size_t reply_size) {});

  // Check results
  EXPECT_EQ(on_listen_called, true);
}

// Tests that SetStreamHandler with a null handler unregisters the handler.
TEST(EventChannelTest, Unregistration) {
  TestBinaryMessenger messenger;
  const std::string channel_name("some_channel");
  const StandardMethodCodec& codec = StandardMethodCodec::GetInstance();
  EventChannel channel(&messenger, channel_name, &codec);

  auto onListen = [](const flutter::EncodableValue* arguments,
                     EventSink<flutter::EncodableValue>* events) {};
  auto onCancel = [](const flutter::EncodableValue* arguments) {};

  flutter::StreamHandler<flutter::EncodableValue> handler(onListen, onCancel);
  channel.SetStreamHandler(&handler);
  EXPECT_EQ(messenger.last_message_handler_channel(), channel_name);
  EXPECT_NE(messenger.last_message_handler(), nullptr);

  channel.SetStreamHandler(nullptr);
  EXPECT_EQ(messenger.last_message_handler_channel(), channel_name);
  EXPECT_EQ(messenger.last_message_handler(), nullptr);
}

// Test of OnCancel callback
TEST(EventChannelTest, Cancel) {
  TestBinaryMessenger messenger;
  const std::string channel_name("some_channel");
  const StandardMethodCodec& codec = StandardMethodCodec::GetInstance();
  EventChannel channel(&messenger, channel_name, &codec);

  auto onListen = [](const flutter::EncodableValue* arguments,
                     EventSink<flutter::EncodableValue>* events) {};

  bool on_cancel_called = false;
  auto onCancel =
      [&on_cancel_called](const flutter::EncodableValue* arguments) {
        on_cancel_called = true;
      };

  flutter::StreamHandler<flutter::EncodableValue> handler(onListen, onCancel);
  channel.SetStreamHandler(&handler);
  EXPECT_EQ(messenger.last_message_handler_channel(), channel_name);
  EXPECT_NE(messenger.last_message_handler(), nullptr);

  // Send dummy cancel message
  MethodCall<flutter::EncodableValue> call("cancel", nullptr);
  auto message = codec.EncodeMethodCall(call);
  messenger.last_message_handler()(
      message->data(), message->size(),
      [](const uint8_t* reply, const size_t reply_size) {});

  // Check results
  EXPECT_EQ(on_cancel_called, true);
}

}  // namespace flutter
