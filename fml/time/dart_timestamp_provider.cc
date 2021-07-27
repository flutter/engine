// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/time/dart_timestamp_provider.h"

#include "dart_tools_api.h"

namespace fml {

DartTimestampProvider::DartTimestampProvider() = default;

DartTimestampProvider::~DartTimestampProvider() = default;

fml::TimePoint DartTimestampProvider::Now() {
  const int64_t ticks = Dart_TimelineGetTicks();
  const int64_t frequency = Dart_TimelineGetTicksFrequency();
  // optimization for the most common case.
  if (frequency != kNanosPerSecond) {
    // convert from frequency base to nanos.
    int64_t seconds = ticks / frequency;
    int64_t leftover_ticks = ticks - (seconds * frequency);
    int64_t result = seconds * kNanosPerSecond;
    result += ((leftover_ticks * kNanosPerSecond) / frequency);
    return fml::TimePoint::FromTicks(result);
  } else {
    return fml::TimePoint::FromTicks(ticks);
  }
}

fml::TimePoint DartTimelineTicksSinceEpoch() {
  return DartTimestampProvider::Instance().Now();
}

}  // namespace fml
