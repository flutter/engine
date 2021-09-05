// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_TESTING_POST_TASK_SYNC_H_
#define FLUTTER_TESTING_POST_TASK_SYNC_H_

#include "flutter/common/task_runners.h"
#include "flutter/fml/task_runner.h"

namespace flutter {
namespace testing {

void PostPlatformTaskSync(const TaskRunners& task_runners,
                          const std::function<void()>& function);

void PostUITaskSync(const TaskRunners& task_runners,
                    const std::function<void()>& function);

void PostRasterTaskSync(const TaskRunners& task_runners,
                        const std::function<void()>& function);

void PostIOTaskSync(const TaskRunners& task_runners,
                    const std::function<void()>& function);

void PostTaskSync(fml::TaskRunner& task_runner,
                  const std::function<void()>& function);

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_TESTING_POST_TASK_SYNC_H_
