// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_DARWIN_CONCURRENT_MESSAGE_LOOP_DARWIN_H_
#define FLUTTER_FML_PLATFORM_DARWIN_CONCURRENT_MESSAGE_LOOP_DARWIN_H_

#include <dispatch/dispatch.h>

#include <mutex>

#include "flutter/fml/macros.h"
#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/synchronization/thread_annotations.h"

namespace fml {

class ConcurrentMessageLoopDarwin : public MessageLoopImpl {
 private:
  dispatch_queue_t queue_;
  std::mutex timer_mutex_;
  dispatch_source_t timer_ FML_GUARDED_BY(timer_mutex_);

  ConcurrentMessageLoopDarwin();

  ~ConcurrentMessageLoopDarwin() override;

  // |fml::MessageLoopImpl|
  void Run() override;

  // |fml::MessageLoopImpl|
  void Terminate() override;

  // |fml::MessageLoopImpl|
  void WakeUp(fml::TimePoint time_point) override;

  static void OnTimerFire(void* user_data);

  void OnTimerFire();

  FML_FRIEND_MAKE_REF_COUNTED(ConcurrentMessageLoopDarwin);
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(ConcurrentMessageLoopDarwin);
  FML_DISALLOW_COPY_AND_ASSIGN(ConcurrentMessageLoopDarwin);
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_DARWIN_CONCURRENT_MESSAGE_LOOP_DARWIN_H_
