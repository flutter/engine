// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/vsync_waiter.h"

namespace shell {

VsyncWaiter::VsyncWaiter(blink::TaskRunners task_runners)
    : task_runners_(std::move(task_runners)) {}

VsyncWaiter::~VsyncWaiter() = default;

}  // namespace shell
