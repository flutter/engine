// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_VSYNC_WAITER_H_
#define FLUTTER_SHELL_COMMON_VSYNC_WAITER_H_

#include <functional>
#include <memory>

#include "flutter/common/task_runners.h"
#include "lib/fxl/time/time_point.h"

namespace shell {

class VsyncWaiter {
 public:
  using Callback = std::function<void(fxl::TimePoint frame_start_time,
                                      fxl::TimePoint frame_target_time)>;

  virtual void AsyncWaitForVsync(Callback callback) = 0;

  virtual ~VsyncWaiter();

 protected:
  const blink::TaskRunners task_runners_;

  VsyncWaiter(blink::TaskRunners task_runners);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_COMMON_VSYNC_WAITER_H_
