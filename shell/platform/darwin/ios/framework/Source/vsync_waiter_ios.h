// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_VSYNC_WAITER_IOS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_VSYNC_WAITER_IOS_H_

#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/common/vsync_waiter.h"
#include "lib/fxl/macros.h"

@class VSyncClient;

namespace shell {

class VsyncWaiterIOS : public VsyncWaiter {
 public:
  VsyncWaiterIOS(blink::TaskRunners task_runners);

  ~VsyncWaiterIOS() override;

  void AsyncWaitForVsync(Callback callback) override;

 private:
  Callback callback_;
  fml::scoped_nsobject<VSyncClient> client_;

  FXL_DISALLOW_COPY_AND_ASSIGN(VsyncWaiterIOS);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_VSYNC_WAITER_IOS_H_
