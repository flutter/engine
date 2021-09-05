// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/post_task_sync.h"

#include "flutter/fml/synchronization/waitable_event.h"

namespace flutter {
namespace testing {

void PostPlatformTaskSync(const TaskRunners& task_runners,
                          const std::function<void()>& function) {
  PostTaskSync(*task_runners.GetPlatformTaskRunner(), function);
}

void PostUITaskSync(const TaskRunners& task_runners,
                    const std::function<void()>& function) {
  PostTaskSync(*task_runners.GetUITaskRunner(), function);
}

void PostRasterTaskSync(const TaskRunners& task_runners,
                        const std::function<void()>& function) {
  PostTaskSync(*task_runners.GetRasterTaskRunner(), function);
}

void PostIOTaskSync(const TaskRunners& task_runners,
                    const std::function<void()>& function) {
  PostTaskSync(*task_runners.GetIOTaskRunner(), function);
}

void PostTaskSync(fml::TaskRunner& task_runner,
                  const std::function<void()>& function) {
  fml::AutoResetWaitableEvent latch;
  task_runner.PostTask([&] {
    function();
    latch.Signal();
  });
  latch.Wait();
}

}  // namespace testing
}  // namespace flutter
