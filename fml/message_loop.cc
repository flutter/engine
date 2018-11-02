// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/message_loop.h"

#include <utility>

#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/task_runner.h"
#include "flutter/fml/thread_local.h"

namespace fml {

FML_THREAD_LOCAL ThreadLocal tls_message_loop([](intptr_t value) {
  delete reinterpret_cast<RefPtr<MessageLoop>*>(value);
});

MessageLoop& MessageLoop::GetCurrent() {
  auto loop = reinterpret_cast<RefPtr<MessageLoop>*>(tls_message_loop.Get());
  FML_CHECK(loop != nullptr)
      << "MessageLoop::EnsureInitializedForCurrentThread was not called on "
         "this thread prior to message loop use.";
  return *loop->get();
}

void MessageLoop::EnsureInitializedForCurrentThread() {
  if (tls_message_loop.Get() != 0) {
    // Already initialized.
    return;
  }
  auto message_loop_ptr = new RefPtr<MessageLoop>(nullptr);
  *message_loop_ptr = MakeRefCounted<MessageLoop>();
  tls_message_loop.Set(reinterpret_cast<intptr_t>(message_loop_ptr));
}

bool MessageLoop::IsInitializedForCurrentThread() {
  return tls_message_loop.Get() != 0;
}

MessageLoop::MessageLoop() {
  PushMessageLoop();
}

MessageLoop::~MessageLoop() = default;

void MessageLoop::Run(fml::closure on_done) {
  FML_DCHECK(impls_.size() != 0);
  if (impls_.top()->DidRun()) {
    PushMessageLoop();
  }
  auto impl_to_run = impls_.top();
  if (on_done) {
    impl_to_run->PostTask(on_done, TimePoint::Now());
  }
  impl_to_run->DoRun();
}

size_t MessageLoop::GetActivationCount() const {
  return impls_.size();
}

void MessageLoop::Terminate() {
  FML_DCHECK(impls_.size() != 0);

  // Pop the message loop on top of the activation stack.
  auto loop_to_terminate = impls_.top();
  impls_.pop();
  loop_to_terminate->DoTerminate();

  // If there is another activation, flush its tasks and rearm timers.
  // If there are no more loops, push an inactive loop so that tasks may be
  // posted onto it.
  if (impls_.size() != 0) {
    impls_.top()->RunExpiredTasksNow();
  } else {
    PushMessageLoop();
  }
}

fml::RefPtr<fml::TaskRunner> MessageLoop::GetTaskRunner() {
  return fml::MakeRefCounted<fml::TaskRunner>(Ref(this));
}

fml::RefPtr<MessageLoopImpl> MessageLoop::GetLoopImpl() const {
  FML_CHECK(impls_.size() > 0);
  return impls_.top();
}

void MessageLoop::AddTaskObserver(intptr_t key, fml::closure callback) {
  GetLoopImpl()->AddTaskObserver(key, callback);
}

void MessageLoop::RemoveTaskObserver(intptr_t key) {
  GetLoopImpl()->RemoveTaskObserver(key);
}

void MessageLoop::RunExpiredTasksNow() {
  GetLoopImpl()->RunExpiredTasksNow();
}

void MessageLoop::PushMessageLoop() {
  auto loop_impl = MessageLoopImpl::Create();
  FML_CHECK(loop_impl);
  impls_.push(std::move(loop_impl));
}

}  // namespace fml
