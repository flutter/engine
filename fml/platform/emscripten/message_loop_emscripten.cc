// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/emscripten/message_loop_emscripten.h"

#include <emscripten.h>

namespace fml {

MessageLoopEmscripten::MessageLoopEmscripten() = default;

// |MessageLoopImpl|
MessageLoopEmscripten::~MessageLoopEmscripten() = default;

// |MessageLoopImpl|
void MessageLoopEmscripten::Run() {
  // Nothing to do.
}

// |MessageLoopImpl|
void MessageLoopEmscripten::Terminate() {
  // Nothing to do.
}

// |MessageLoopImpl|
void MessageLoopEmscripten::WakeUp(fml::TimePoint time_point) {
  const auto timeout = std::max<float>(
      0.0f, (time_point - fml::TimePoint::Now()).ToMillisecondsF());
  // This is only suitable for a single threaded emscripten configuration. A
  // linux like message loop impl will be used on those configurations.
  emscripten_async_call(
      [](void* arg) {
        reinterpret_cast<MessageLoopEmscripten*>(arg)->RunExpiredTasksNow();
      },
      this, timeout);
}

}  // namespace fml
