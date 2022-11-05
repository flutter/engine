// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/rasterizer_sleep_strategy.h"

namespace flutter {

// A simple threshold. Can change to fancier approaches later.
static const int kMaxHistory = 10;
static const int kHistoryThresh = 2;
// a bit more than enough to allow errors
static const fml::TimeDelta kSafeMargin = fml::TimeDelta::FromMicroseconds(500);

std::optional<fml::TimePoint> RasterizerSleepStrategy::Handle(
    fml::TimePoint now,
    fml::TimePoint vsync_target_time,
    fml::TimeDelta frame_budget) {
  int curr_latency = static_cast<int>(
      (now - vsync_target_time + frame_budget * 2).ToMicroseconds() /
      frame_budget.ToMicroseconds());

  // naive heuristics currently
  int history_larger_latency_count = 0;
  for (int history_latency : history_latencies_) {
    if (history_latency > curr_latency) {
      history_larger_latency_count++;
    }
  }
  bool should_sleep = history_larger_latency_count >= kHistoryThresh;

  // Again, this is very naive and can be fancier later
  int expect_latency = curr_latency + 1;
  // we want to wake up at the *beginning* of that vsync interval
  fml::TimePoint wakeup_time =
      vsync_target_time + frame_budget * (expect_latency - 2) + kSafeMargin;

  {
    history_latencies_.push_back(curr_latency);
    while (history_latencies_.size() > kMaxHistory) {
      history_latencies_.pop_front();
    }
  }

  return should_sleep ? std::optional(wakeup_time) : std::nullopt;
}

}  // namespace flutter
