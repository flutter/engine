// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/resource_cache_limit_calculator.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(ResourceCacheLimitCalculatorTest, UpdateResourceCacheBytes) {
  ResourceCacheLimitCalculator calculator(800U);
  calculator.UpdateResourceCacheBytes(0, 100U);
  calculator.UpdateResourceCacheBytes(1, 200U);
  EXPECT_EQ(calculator.GetResourceCacheBytes(0), static_cast<size_t>(100U));
  EXPECT_EQ(calculator.GetResourceCacheBytes(1), static_cast<size_t>(200U));

  calculator.UpdateResourceCacheBytes(0, 300U);
  calculator.UpdateResourceCacheBytes(1, 400U);
  EXPECT_EQ(calculator.GetResourceCacheBytes(0), static_cast<size_t>(300U));
  EXPECT_EQ(calculator.GetResourceCacheBytes(1), static_cast<size_t>(400U));
}

TEST(ResourceCacheLimitCalculatorTest, RemoveResourceCacheBytes) {
  ResourceCacheLimitCalculator calculator(800U);
  EXPECT_EQ(calculator.GetResourceCacheBytes(0), static_cast<size_t>(0U));

  calculator.UpdateResourceCacheBytes(0, 100U);
  EXPECT_EQ(calculator.GetResourceCacheBytes(0), static_cast<size_t>(100U));

  calculator.RemoveResourceCacheBytes(0);
  EXPECT_EQ(calculator.GetResourceCacheBytes(0), static_cast<size_t>(0U));
}

TEST(ResourceCacheLimitCalculatorTest, GetResourceCacheMaxBytes) {
  ResourceCacheLimitCalculator calculator(800U);
  calculator.UpdateResourceCacheBytes(0, 100U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(100U));

  calculator.UpdateResourceCacheBytes(1, 200U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(300U));

  calculator.UpdateResourceCacheBytes(2, 300U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(600U));

  calculator.UpdateResourceCacheBytes(3, 400U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(800U));

  calculator.RemoveResourceCacheBytes(2);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(700U));

  calculator.RemoveResourceCacheBytes(1);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(500U));
}

TEST(ResourceCacheLimitCalculatorTest, UpdateMaxBytesThreshold) {
  ResourceCacheLimitCalculator calculator(800U);
  calculator.UpdateResourceCacheBytes(0, 1000U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(800U));

  calculator.UpdateMaxBytesThreshold(1200U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(1000U));

  calculator.UpdateMaxBytesThreshold(-1);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(1000U));

  calculator.UpdateMaxBytesThreshold(500U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(500U));
}

}  // namespace testing
}  // namespace flutter
