// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/shell/common/rasterizer_sleep_strategy.h"

#include <memory>
#include <optional>

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(RasterizerSleepStrategyTest, WhenConstantlyOneFrameShouldNeverSleep) {
  const auto frame_budget = fml::TimeDelta::FromMicroseconds(16667);
  RasterizerSleepStrategy strategy;

  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(16000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(16667)),
                            frame_budget),
            std::nullopt);
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(32000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(33333)),
                            frame_budget),
            std::nullopt);
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(48000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(50000)),
                            frame_budget),
            std::nullopt);
}

TEST(RasterizerSleepStrategyTest, WhenConstantlyTwoFramesShouldNeverSleep) {
  const auto frame_budget = fml::TimeDelta::FromMicroseconds(16667);
  RasterizerSleepStrategy strategy;

  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(16000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(0)),
                            frame_budget),
            std::nullopt);
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(32000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(16667)),
                            frame_budget),
            std::nullopt);
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(48000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(33333)),
                            frame_budget),
            std::nullopt);
}

TEST(RasterizerSleepStrategyTest, WhenConstantlyThreeFramesShouldNeverSleep) {
  const auto frame_budget = fml::TimeDelta::FromMicroseconds(16667);
  RasterizerSleepStrategy strategy;

  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(16000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(-16667)),
                            frame_budget),
            std::nullopt);
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(32000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(0)),
                            frame_budget),
            std::nullopt);
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(48000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(16667)),
                            frame_budget),
            std::nullopt);
}

TEST(RasterizerSleepStrategyTest, WhenOneLessFrameShouldSleep) {
  const auto frame_budget = fml::TimeDelta::FromMicroseconds(16667);
  RasterizerSleepStrategy strategy;

  // normally it is 32ms delay from vsync start to raster end
  // i.e. 16ms from vsync *target* time to raster end
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(1000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(0)),
                            frame_budget),
            std::nullopt);
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(18000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(16667)),
                            frame_budget),
            std::nullopt);

  // suddenly this rasterization is too fast
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(33000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(33333)),
                            frame_budget),
            std::optional(fml::TimePoint::FromEpochDelta(
                fml::TimeDelta::FromMicroseconds(33333 + 500))));

  // ... and then it becomes normal delay again
  EXPECT_EQ(strategy.Handle(fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(52000)),
                            fml::TimePoint::FromEpochDelta(
                                fml::TimeDelta::FromMicroseconds(50000)),
                            frame_budget),
            std::nullopt);
}

}  // namespace testing
}  // namespace flutter
