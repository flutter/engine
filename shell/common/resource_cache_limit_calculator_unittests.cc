// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/resource_cache_limit_calculator.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(ResourceCacheLimitCalculatorTest, UpdateResourceCacheBytes) {
  ResourceCacheLimitCalculator calculator(800U);
  int key1 = 1;
  int key2 = 2;
  calculator.UpdateResourceCacheBytes(&key1, 100U);
  calculator.UpdateResourceCacheBytes(&key2, 200U);
  EXPECT_EQ(calculator.GetResourceCacheBytes(&key1), static_cast<size_t>(100U));
  EXPECT_EQ(calculator.GetResourceCacheBytes(&key2), static_cast<size_t>(200U));

  calculator.UpdateResourceCacheBytes(&key1, 300U);
  calculator.UpdateResourceCacheBytes(&key2, 400U);
  EXPECT_EQ(calculator.GetResourceCacheBytes(&key1), static_cast<size_t>(300U));
  EXPECT_EQ(calculator.GetResourceCacheBytes(&key2), static_cast<size_t>(400U));
}

TEST(ResourceCacheLimitCalculatorTest, RemoveResourceCacheBytes) {
  ResourceCacheLimitCalculator calculator(800U);
  int key = 0;
  EXPECT_EQ(calculator.GetResourceCacheBytes(&key), static_cast<size_t>(0U));

  calculator.UpdateResourceCacheBytes(&key, 100U);
  EXPECT_EQ(calculator.GetResourceCacheBytes(&key), static_cast<size_t>(100U));

  calculator.RemoveResourceCacheBytes(&key);
  EXPECT_EQ(calculator.GetResourceCacheBytes(&key), static_cast<size_t>(0U));
}

TEST(ResourceCacheLimitCalculatorTest, GetResourceCacheMaxBytes) {
  ResourceCacheLimitCalculator calculator(800U);
  int key1 = 1;
  int key2 = 2;
  int key3 = 3;
  int key4 = 4;
  calculator.UpdateResourceCacheBytes(&key1, 100U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(100U));

  calculator.UpdateResourceCacheBytes(&key2, 200U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(300U));

  calculator.UpdateResourceCacheBytes(&key3, 300U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(600U));

  calculator.UpdateResourceCacheBytes(&key4, 400U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(800U));

  calculator.RemoveResourceCacheBytes(&key3);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(700U));

  calculator.RemoveResourceCacheBytes(&key2);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(500U));
}

TEST(ResourceCacheLimitCalculatorTest, UpdateMaxBytesThreshold) {
  ResourceCacheLimitCalculator calculator(800U);
  calculator.UpdateResourceCacheBytes(0, 1000U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(800U));

  calculator.UpdateMaxBytesThreshold(1200U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(1000U));

  calculator.UpdateMaxBytesThreshold(0U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(1000U));

  calculator.UpdateMaxBytesThreshold(500U);
  EXPECT_EQ(calculator.GetResourceCacheMaxBytes(), static_cast<size_t>(500U));
}

}  // namespace testing
}  // namespace flutter
