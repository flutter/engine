// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_SINK_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_SINK_H_

namespace flutter {

// Event callback. Supports dual use: Producers of events to be sent to Flutter act as clients of
// this interface for sending events. Consumers of events sent from Flutter implement this
// interface for handling received events.
template <typename T>
class EventSink {
 public:
  EventSink() = default;
  virtual ~EventSink() = default;

  // Prevent copying.
  EventSink(EventSink const&) = delete;
  EventSink& operator=(EventSink const&) = delete;

  // Consumes end of stream. Ensuing calls to Success(T) or
  // Error(String, String, Object)}, if any, are ignored.
  void EndOfStream() { EndOfStreamInternal(); }

  // Consumes a successful event.
  void Success(T* event = nullptr) {
    SuccessInternal(event);
  }

  // Consumes an error event.
  void Error(const std::string& error_code,
             const std::string& error_message = "",
             T* error_details = nullptr) {
    ErrorInternal(error_code, error_message, error_details);
  }

 protected:
  // Implementation of the public interface, to be provided by subclasses.
  virtual void EndOfStreamInternal() = 0;

  // Implementation of the public interface, to be provided by subclasses.
  virtual void SuccessInternal(T* event = nullptr) = 0;

  // Implementation of the public interface, to be provided by subclasses.
  virtual void ErrorInternal(const std::string& error_code,
                             const std::string& error_message,
                             T* error_details) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_SINK_H_
