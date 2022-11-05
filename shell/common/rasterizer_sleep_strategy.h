// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_RASTERIZER_SLEEP_STRATEGY_H_
#define SHELL_COMMON_RASTERIZER_SLEEP_STRATEGY_H_

#include <deque>
#include <memory>
#include <optional>

#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/time/time_point.h"

namespace flutter {

class RasterizerSleepStrategy {
 public:
  std::optional<fml::TimePoint> Handle(fml::TimePoint now,
                                       fml::TimePoint vsync_target_time,
                                       fml::TimeDelta frame_budget);

 private:
  std::deque<int> history_latencies_;
};

}  // namespace flutter

#endif  // SHELL_COMMON_RASTERIZER_SLEEP_STRATEGY_H_
