// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/darwin/string_range_sanitization.h"
#include "gtest/gtest.h"

TEST(StringRangeSanitizationTest, CanHandleUnicode) {
  auto result = fml::RangeForCharacterAtIndex(@"😠", 1);
  EXPECT_EQ(result.location, 0UL);
  EXPECT_EQ(result.length, 2UL);
}

TEST(StringRangeSanitizationTest, CanHandleUnicodeRange) {
  auto result = fml::RangeForCharactersInRange(@"😠", NSMakeRange(1, 0));
  EXPECT_EQ(result.location, 0UL);
  EXPECT_EQ(result.length, 0UL);
}
