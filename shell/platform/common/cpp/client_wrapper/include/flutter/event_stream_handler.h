// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_STREAM_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_STREAM_HANDLER_H_

#include "event_sink.h"

namespace flutter {

template <typename T>
struct StreamHandlerError {
  const std::string& error_code;
  const std::string& error_message;
  const T* error_details;

  StreamHandlerError(const std::string& error_code,
                     const std::string& error_message,
                     const T* error_details)
    : error_code(error_code),
      error_message(error_message),
      error_details(error_details) {}
};

// Handler of stream setup and tear-down requests.
// Implementations must be prepared to accept sequences of alternating calls to
// onListen() and onCancel(). Implementations should ideally consume no
// resources when the last such call is not onListen(). In typical situations,
// this means that the implementation should register itself with
// platform-specific event sources onListen() and deregister again onCancel().
template <typename T>
struct StreamHandler {
  // Handles a request to set up an event stream. Returns error if representing
  // an unsuccessful outcome of invoking the method, possibly nullptr.
  // |arguments| is stream configuration arguments and
  // |events| is an EventSink for emitting events to the Flutter receiver.
  using OnListen = std::function<std::unique_ptr<StreamHandlerError<T>>(
      const T* arguments,
      std::unique_ptr<EventSink<T>>&& events)>;

  // Handles a request to tear down the most recently created event stream.
  // Returns error if representing an unsuccessful outcome of invoking the
  // method, possibly nullptr.
  // |arguments| is stream configuration arguments.
  using OnCancel = 
      std::function<std::unique_ptr<StreamHandlerError<T>>(const T* arguments)>;

  OnListen onListen;
  OnCancel onCancel;

  StreamHandler(OnListen const& onListen, OnCancel const& onCancel)
      : onListen(onListen), onCancel(onCancel) {}
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_STREAM_HANDLER_H_
