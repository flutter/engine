// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_EMSCRIPTEN_MESSAGE_LOOP_EMSCRIPTEN_H_
#define FLUTTER_FML_PLATFORM_EMSCRIPTEN_MESSAGE_LOOP_EMSCRIPTEN_H_

#include "flutter/fml/message_loop_impl.h"

namespace fml {

class MessageLoopEmscripten final : public MessageLoopImpl {
 public:
  MessageLoopEmscripten();

  MessageLoopEmscripten(const MessageLoopEmscripten&) = delete;

  MessageLoopEmscripten& operator=(const MessageLoopEmscripten&) = delete;

  // |MessageLoopImpl|
  ~MessageLoopEmscripten() override;

  // |MessageLoopImpl|
  void Run() override;

  // |MessageLoopImpl|
  void Terminate() override;

  // |MessageLoopImpl|
  void WakeUp(fml::TimePoint time_point) override;
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_EMSCRIPTEN_MESSAGE_LOOP_EMSCRIPTEN_H_
