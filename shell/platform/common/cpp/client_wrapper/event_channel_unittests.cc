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
                      EventSink<flutter::EncodableValue>* event_sink) {
    event_sink->Success();
    auto message = flutter::EncodableValue(flutter::EncodableMap{
        {flutter::EncodableValue("message"),
         flutter::EncodableValue("Test from Event Channel")}});
    event_sink->Success(&message);
    event_sink->Error("Event Channel Error Code", "Error Message", nullptr);
    event_sink->EndOfStream();
    on_listen_called = true;
  };

  bool on_cancel_called = false;
  auto onCancel =
      [&on_cancel_called](const flutter::EncodableValue* arguments) {
        on_cancel_called = true;
      };
  channel.SetStreamHandler(
      flutter::StreamHandler<flutter::EncodableValue>(onListen, onCancel));

  EXPECT_EQ(messenger.last_message_handler_channel(), channel_name);
  EXPECT_NE(messenger.last_message_handler(), nullptr);

  // Send a test message to trigger the handler test assertions.
  MethodCall<EncodableValue> call("listen", nullptr);
  auto message = codec.EncodeMethodCall(call);
  messenger.last_message_handler()(
      message->data(), message->size(),
      [](const uint8_t* reply, const size_t reply_size) {});
  EXPECT_EQ(on_listen_called, true);

  // FIXME: add onCancel test scenario
  // EXPECT_EQ(on_cancel_called, true);
}

// Tests that SetStreamHandler with a null handler unregisters the handler.
TEST(EventChannelTest, Unregistration) {
  TestBinaryMessenger messenger;
  const std::string channel_name("some_channel");
  EventChannel<flutter::EncodableValue> channel(
      &messenger, channel_name, &flutter::StandardMethodCodec::GetInstance());

  auto onListen = [](const flutter::EncodableValue* arguments,
                     EventSink<flutter::EncodableValue>* event_sink) {};
  auto onCancel = [](const flutter::EncodableValue* arguments) {};
  channel.SetStreamHandler(
      flutter::StreamHandler<flutter::EncodableValue>(onListen, onCancel));
  EXPECT_EQ(messenger.last_message_handler_channel(), channel_name);
  EXPECT_NE(messenger.last_message_handler(), nullptr);

  // channel.SetStreamHandler(std::nullopt);
  // EXPECT_EQ(messenger.last_message_handler_channel(), channel_name);
  // EXPECT_EQ(messenger.last_message_handler(), nullptr);
}

}  // namespace flutter
