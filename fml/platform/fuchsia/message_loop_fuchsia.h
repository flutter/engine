// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_FUCHSIA_MESSAGE_LOOP_FUCHSIA_H_
#define FLUTTER_FML_PLATFORM_FUCHSIA_MESSAGE_LOOP_FUCHSIA_H_

#include "flutter/fml/macros.h"
#include "flutter/fml/message_loop_impl.h"

#include <zircon/syscalls.h>
#include <lib/async/cpp/wait.h>
#include <lib/async-loop/cpp/loop.h>

namespace fml {

class MessageLoopFuchsia : public MessageLoopImpl {
 private:
  MessageLoopFuchsia();

  ~MessageLoopFuchsia() override;

  void Run() override;

  void Terminate() override;

  void WakeUp(fml::TimePoint time_point) override;

  zx_handle_t timer_;

  async::Wait* timer_wait_;
  async::Loop loop_;

  bool running_;

  FML_FRIEND_MAKE_REF_COUNTED(MessageLoopFuchsia);
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(MessageLoopFuchsia);
  FML_DISALLOW_COPY_AND_ASSIGN(MessageLoopFuchsia);
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_FUCHSIA_MESSAGE_LOOP_FUCHSIA_H_
