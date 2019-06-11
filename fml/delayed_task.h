// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_DELAYED_TASK_H_
#define FLUTTER_FML_DELAYED_TASK_H_

#include "flutter/fml/closure.h"
#include "flutter/fml/time/time_point.h"

namespace fml {

struct DelayedTask {
  int order;
  fml::closure task;
  fml::TimePoint target_time;

  DelayedTask(int p_order, fml::closure p_task, fml::TimePoint p_target_time);

  DelayedTask(const DelayedTask& other);

  ~DelayedTask();
};

struct DelayedTaskCompare {
  bool operator()(const DelayedTask& a, const DelayedTask& b) {
    return a.target_time == b.target_time ? a.order > b.order
                                          : a.target_time > b.target_time;
  }
};

}  // namespace fml

#endif  // FLUTTER_FML_DELAYED_TASK_H_
