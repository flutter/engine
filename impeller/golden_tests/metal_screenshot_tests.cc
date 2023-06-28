// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"

#include "impeller/golden_tests/metal_screenshot.h"

namespace impeller {
namespace testing {

std::unique_ptr<MetalScreenshot> MetalScreenshotTestConstructor(
    CGImageRef image) {
  return std::unique_ptr<MetalScreenshot>(new MetalScreenshot(image));
};

TEST(MetalScreenshotTests, NullScreenshot) {
  auto shot = MetalScreenshotTestConstructor(nullptr);
  ASSERT_EQ(shot->GetBytes(), nullptr);
  ASSERT_EQ(shot->GetWidth(), 0u);
  ASSERT_EQ(shot->GetHeight(), 0u);
  ASSERT_EQ(shot->WriteToPNG("foo"), false);
}

}  // namespace testing
}  // namespace impeller
