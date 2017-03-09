// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/message_loop_impl.h"
#include "lib/ftl/build_config.h"

#if OS_MACOSX

#include "flutter/fml/platform/darwin/message_loop_darwin.h"
using PlatformMessageLoopImpl = fml::MessageLoopDarwin;

#elif OS_ANDROID

#include "flutter/fml/platform/android/message_loop_android.h"
using PlatformMessageLoopImpl = fml::MessageLoopAndroid;

#else

#error This platform does not have a message loop implementation.

#endif

namespace fml {

ftl::RefPtr<MessageLoopImpl> MessageLoopImpl::Create() {
  return ftl::MakeRefCounted<::PlatformMessageLoopImpl>();
}

MessageLoopImpl::MessageLoopImpl() : order_(0) {}

MessageLoopImpl::~MessageLoopImpl() = default;

void MessageLoopImpl::PostTask(ftl::Closure task, ftl::TimePoint target_time) {
  FTL_DCHECK(task != nullptr);
  WakeUp(RegisterTaskAndGetNextWake(task, target_time));
}

void MessageLoopImpl::RunExpiredTasksNow() {
  WakeUp(RunExpiredTasksAndGetNextWake());
}

void MessageLoopImpl::SetTaskObserver(MessageLoop::TaskObserver observer) {
  FTL_DCHECK(MessageLoop::GetCurrent().GetLoopImpl().get() == this)
      << "Message loop task observer must be set on the same thread as the "
         "loop.";
  task_observer_ = observer;
}

ftl::TimePoint MessageLoopImpl::RegisterTaskAndGetNextWake(
    ftl::Closure task,
    ftl::TimePoint target_time) {
  FTL_DCHECK(task != nullptr);
  ftl::MutexLocker lock(&delayed_tasks_mutex_);
  delayed_tasks_.push({++order_, std::move(task), target_time});
  return delayed_tasks_.top().target_time;
}

ftl::TimePoint MessageLoopImpl::RunExpiredTasksAndGetNextWake() {
  std::vector<ftl::Closure> invocations;

  {
    ftl::MutexLocker lock(&delayed_tasks_mutex_);

    if (delayed_tasks_.empty()) {
      return ftl::TimePoint::Max();
    }

    auto now = ftl::TimePoint::Now();
    while (!delayed_tasks_.empty()) {
      const auto& top = delayed_tasks_.top();
      if (top.target_time > now) {
        break;
      }
      invocations.emplace_back(std::move(top.task));
      delayed_tasks_.pop();
    }
  }

  for (const auto& invocation : invocations) {
    invocation();
    if (task_observer_) {
      task_observer_();
    }
  }

  ftl::MutexLocker lock(&delayed_tasks_mutex_);
  return delayed_tasks_.empty() ? ftl::TimePoint::Max()
                                : delayed_tasks_.top().target_time;
}

}  // namespace fml
