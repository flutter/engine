// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// FLUTTER_NOLINT

#include "flutter/shell/common/vsync_waiter_fallback.h"

#include "flutter/fml/logging.h"

namespace flutter {
namespace {

static fml::TimePoint SnapToNextTick(fml::TimePoint value,
                                     fml::TimePoint tick_phase,
                                     fml::TimeDelta tick_interval) {
  fml::TimeDelta offset = (tick_phase - value) % tick_interval;
  if (offset != fml::TimeDelta::Zero())
    offset = offset + tick_interval;
  return value + offset;
}

}  // namespace

VsyncWaiterFallback::VsyncWaiterFallback(TaskRunners task_runners,
                                         float display_refresh_rate)
    : VsyncWaiter(std::move(task_runners)),
      phase_(fml::TimePoint::Now()),
      display_refresh_rate_(display_refresh_rate) {}

VsyncWaiterFallback::VsyncWaiterFallback(TaskRunners task_runners)
    : VsyncWaiterFallback(std::move(task_runners),
                          VsyncWaiter::kUnknownRefreshRateFPS) {}

VsyncWaiterFallback::~VsyncWaiterFallback() = default;

// |VsyncWaiter|
void VsyncWaiterFallback::AwaitVSync() {
  constexpr fml::TimeDelta kSingleFrameInterval =
      fml::TimeDelta::FromSecondsF(1.0 / 60.0);

  auto next =
      SnapToNextTick(fml::TimePoint::Now(), phase_, kSingleFrameInterval);

  FireCallback(next, next + kSingleFrameInterval);
}

float VsyncWaiterFallback::GetDisplayRefreshRate() const {
  return display_refresh_rate_;
}

}  // namespace flutter
