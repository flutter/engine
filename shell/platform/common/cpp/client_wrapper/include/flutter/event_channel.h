// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_CHANNEL_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_CHANNEL_H_

#include <iostream>
#include <string>

#include "binary_messenger.h"
#include "engine_method_result.h"
#include "message_codec.h"
#include "method_call.h"
#include "method_codec.h"
#include "method_result.h"

namespace flutter {

template <typename T>
using EventSink = std::function<void(const T *events)>;

template <typename T>
struct StreamHandler {
  std::function<MethodResult<T> *(const T *arguments,
                                  const EventSink<T> *event)>
      onListen;
  std::function<MethodResult<T> *(const T *arguments)> onCancel;
};

template <typename T>
class EventChannel {
 public:
  EventChannel(BinaryMessenger *messenger, const std::string &name,
               const MethodCodec<T> *codec,
               const MessageCodec<T> *message_codec)
      : messenger_(messenger),
        name_(name),
        codec_(codec),
        message_code_(message_codec) {}
  ~EventChannel() {}

  // Prevent copying.
  EventChannel(EventChannel const &) = delete;
  EventChannel &operator=(EventChannel const &) = delete;

  void SetStreamHandler(StreamHandler<T> stream_handler) {
    stream_handler_ = stream_handler;
    const auto message_codec = message_code_;
    const auto messenger = messenger_;
    std::string channel_name = name_;
    EventSink<T> sink = [messenger, channel_name,
                         message_codec](const T *events) {
      std::unique_ptr<std::vector<uint8_t>> message =
          message_codec->EncodeMessage(events);
      messenger->Send(channel_name, message->data(), message->size());
    };

    const auto *codec = codec_;
    BinaryMessageHandler binary_handler = [&, sink, codec, channel_name](
                                              const uint8_t *message,
                                              const size_t message_size,
                                              BinaryReply reply) {
      auto result =
          std::make_unique<EngineMethodResult<T>>(std::move(reply), codec);
      std::unique_ptr<MethodCall<T>> method_call =
          codec->DecodeMethodCall(message, message_size);
      if (!method_call) {
        std::cerr << "Unable to construct method call from message on channel "
                  << channel_name << std::endl;
        result->NotImplemented();
        return;
      }
      if (method_call->method_name().compare("listen") == 0) {
        stream_handler_.onListen(method_call->arguments(), &sink);
        result->Success(nullptr);
        return;
      } else if (method_call->method_name().compare("cancel") == 0) {
        stream_handler_.onCancel(method_call->arguments());
        result->Success(nullptr);
        return;
      } else {
        result->NotImplemented();
      }
    };

    messenger_->SetMessageHandler(name_, std::move(binary_handler));
  }

 private:
  BinaryMessenger *messenger_;
  std::string name_;
  const MethodCodec<T> *codec_;
  const MessageCodec<T> *message_code_;
  StreamHandler<T> stream_handler_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_CHANNEL_H_
