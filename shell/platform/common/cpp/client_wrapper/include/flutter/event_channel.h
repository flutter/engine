// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_CHANNEL_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_CHANNEL_H_

#include <iostream>
#include <string>

#include "binary_messenger.h"
#include "engine_method_result.h"
#include "event_stream_handler.h"
#include "event_sink.h"

static constexpr char kOnListenMethod[] = "listen";
static constexpr char kOnCancelMethod[] = "cancel";

namespace flutter {

// A named channel for communicating with the Flutter application using 
// asynchronous event streams. Incoming requests for event stream setup are
// decoded from binary on receipt, and C++ responses and events are encoded into
// binary before being transmitted back to Flutter. The MethodCodec used must be
// compatible with the one used by the Flutter application. This can be achieved
// by creating an EventChannel
// ("https://docs.flutter.io/flutter/services/EventChannel-class.html")
// counterpart of this channel on the Dart side. The C++ type of stream
// configuration arguments, events, and error details is Object, but only values
// supported by the specified MethodCodec can be used.
//
// The logical identity of the channel is given by its name. Identically named
// channels will interfere with each other's communication.
template <typename T>
class EventChannel {
 public:
  // Creates an instance that sends and receives event handler on the channel
  // named |name|, encoded with |codec| and dispatched via |messenger|.
  EventChannel(BinaryMessenger* messenger,
               const std::string& name,
               const MethodCodec<T>* codec)
      : messenger_(messenger), name_(name), codec_(codec) {}
  ~EventChannel() = default;

  // Prevent copying.
  EventChannel(EventChannel const&) = delete;
  EventChannel& operator=(EventChannel const&) = delete;

  // Registers a stream handler on this channel.
  // If no handler has been registered, any incoming stream setup requests will be handled
  // silently by providing an empty stream.
  void SetStreamHandler(const StreamHandler<T>& handler) const {
    // TODO: The following is required when nullptr
    // can be passed as an argument.
    //if (!handler) { /* <= available for more than C++17 */
    //  messenger_->SetMessageHandler(name_, nullptr);
    //  return;
    //}

    const auto* codec = codec_;
    const std::string channel_name = name_;
    const auto* messenger = messenger_;
    EventSinkImplementation* current_sink = current_sink_;
    BinaryMessageHandler binary_handler = [handler, codec, channel_name,
                                           current_sink, messenger](
                                              const uint8_t* message,
                                              const size_t message_size,
                                              BinaryReply reply) mutable {
      std::unique_ptr<MethodCall<T>> method_call =
          codec->DecodeMethodCall(message, message_size);
      if (!method_call) {
        std::cerr << "Unable to construct method call from message on channel "
                  << channel_name << std::endl;
        return;
      }

      const std::string& method = method_call->method_name();
      if (method.compare(kOnListenMethod) == 0) {
        if (current_sink) {
          std::cerr << "Failed to cancel existing stream: " << channel_name
                    << std::endl;
          handler.onCancel(method_call->arguments());
          delete current_sink;
          current_sink = nullptr;
        }

        current_sink =
            new EventSinkImplementation(messenger, channel_name, codec);
        handler.onListen(method_call->arguments(),
                         static_cast<EventSink<T>*>(current_sink));

        {
          auto result = codec->EncodeSuccessEnvelope();
          uint8_t* buffer = new uint8_t[result->size()];
          std::copy(result->begin(), result->end(), buffer);
          reply(buffer, result->size());
          delete[] buffer;
        }
      } else if (method.compare(kOnCancelMethod) == 0) {
        if (current_sink) {
          handler.onCancel(method_call->arguments());

          auto result = codec->EncodeSuccessEnvelope();
          uint8_t* buffer = new uint8_t[result->size()];
          std::copy(result->begin(), result->end(), buffer);
          reply(buffer, result->size());
          delete current_sink;
          current_sink = nullptr;
        } else {
          auto result = codec->EncodeErrorEnvelope(
              "error", "No active stream to cancel", nullptr);
          uint8_t* buffer = new uint8_t[result->size()];
          std::copy(result->begin(), result->end(), buffer);
          reply(buffer, result->size());
          delete[] buffer;
        }
      } else {
        std::cerr
            << "Unknown event channel method call from message on channel: "
            << channel_name << std::endl;
        reply(nullptr, 0);
        if (current_sink) {
          delete current_sink;
          current_sink = nullptr;
        }
      }
    };
    messenger_->SetMessageHandler(name_, std::move(binary_handler));
  }

 private:
  class EventSinkImplementation : public EventSink<T> {
   public:
    // Creates an instance that EventSink send event on the channel
    // named |name|, encoded with |codec| and dispatched via |messenger|.
    EventSinkImplementation(const BinaryMessenger* messenger,
                            const std::string& name,
                            const MethodCodec<T>* codec)
        : messenger_(messenger), name_(name), codec_(codec) {}
    ~EventSinkImplementation() = default;

    // Prevent copying.
    EventSinkImplementation(EventSinkImplementation const&) = delete;
    EventSinkImplementation& operator=(EventSinkImplementation const&) = delete;

   private:
    const BinaryMessenger* messenger_;
    const std::string name_;
    const MethodCodec<T>* codec_;

   protected:
    void SuccessInternal(T* event = nullptr) override {
      auto result = codec_->EncodeSuccessEnvelope(event);
      uint8_t* buffer = new uint8_t[result->size()];
      std::copy(result->begin(), result->end(), buffer);
      messenger_->Send(name_, buffer, result->size());
      delete[] buffer;
    }

    void ErrorInternal(const std::string& error_code,
                       const std::string& error_message,
                       T* error_details) override {
      auto result =
          codec_->EncodeErrorEnvelope(error_code, error_message, error_details);
      uint8_t* buffer = new uint8_t[result->size()];
      std::copy(result->begin(), result->end(), buffer);
      messenger_->Send(name_, buffer, result->size());
      delete[] buffer;
    }

    void EndOfStreamInternal() override { messenger_->Send(name_, nullptr, 0); }
  };

  BinaryMessenger* messenger_;
  std::string name_;
  const MethodCodec<T>* codec_;
  EventSinkImplementation* current_sink_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_CHANNEL_H_
