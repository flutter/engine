// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/time/chrono_timestamp_provider.h"

#include "flutter/fml/time/dart_timestamp_provider.h"

#include "gtest/gtest.h"

namespace fml {
namespace {

TEST(TimePoint, Control) {
  EXPECT_LT(TimePoint::Min(), ChronoTicksSinceEpoch());
  EXPECT_GT(TimePoint::Max(), ChronoTicksSinceEpoch());
}

TEST(TimePoint, DartClockIsMonotonic) {
  const auto t1 = DartTimelineTicksSinceEpoch();
  const auto t2 = DartTimelineTicksSinceEpoch();
  const auto t3 = DartTimelineTicksSinceEpoch();
  EXPECT_LT(TimePoint::Min(), t1);
  EXPECT_LT(t1, t2);
  EXPECT_LT(t2, t3);
  EXPECT_LT(t3, TimePoint::Max());
}

}  // namespace
}  // namespace fml
