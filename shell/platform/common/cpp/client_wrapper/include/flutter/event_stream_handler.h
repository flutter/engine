// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_STREAM_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_STREAM_HANDLER_H_

#include "event_sink.h"

namespace flutter {

// Handler of stream setup and tear-down requests.
// Implementations must be prepared to accept sequences of alternating calls to
// onListen() and onCancel(). Implementations should ideally consume no resources
// when the last such call is not onListen(). In typical situations,
// this means that the implementation should register itself with
// platform-specific event sources onListen() and deregister again onCancel().
template <typename T>
struct StreamHandler {
  // Handles a request to set up an event stream.
  // |arguments| is stream configuration arguments and
  // |events| is an EventSink for emitting events to the Flutter receiver.
  using OnListen =
      std::function<void(const T* arguments, EventSink<T>* events)>;

  // Handles a request to tear down the most recently created event stream.
  // |arguments| is stream configuration arguments.
  using OnCancel = std::function<void(const T* arguments)>;

  OnListen onListen;
  OnCancel onCancel;

  StreamHandler(OnListen const& onListen, OnCancel const& onCancel)
      : onListen(onListen), onCancel(onCancel) {}
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_STREAM_HANDLER_H_
