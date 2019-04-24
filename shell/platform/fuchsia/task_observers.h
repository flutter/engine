// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOPAZ_RUNTIME_FLUTTER_RUNNER_TASK_OBSERVERS_H_
#define TOPAZ_RUNTIME_FLUTTER_RUNNER_TASK_OBSERVERS_H_

#include <lib/fit/function.h>

namespace flutter_runner {

void ExecuteAfterTaskObservers();

void CurrentMessageLoopAddAfterTaskObserver(intptr_t key,
                                            fit::closure observer);

void CurrentMessageLoopRemoveAfterTaskObserver(intptr_t key);

}  // namespace flutter_runner

#endif  // TOPAZ_RUNTIME_FLUTTER_RUNNER_TASK_OBSERVERS_H_
