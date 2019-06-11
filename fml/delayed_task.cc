// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/delayed_task.h"

namespace fml {

// delayed tasks
DelayedTask::DelayedTask(int p_order,
                         fml::closure p_task,
                         fml::TimePoint p_target_time)
    : order(p_order), task(std::move(p_task)), target_time(p_target_time) {}

DelayedTask::DelayedTask(const DelayedTask& other) = default;

DelayedTask::~DelayedTask() = default;

}  // namespace fml
