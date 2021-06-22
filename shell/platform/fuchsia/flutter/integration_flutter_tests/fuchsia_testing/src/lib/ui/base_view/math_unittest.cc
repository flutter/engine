// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/ui/base_view/math.h"

#include <fuchsia/ui/gfx/cpp/fidl.h>

#include <gtest/gtest.h>

namespace scenic {

using fuchsia::ui::gfx::vec2;

// TEST(MathTest, Vec2Length) { EXPECT_EQ(scenic::Length({3, 4}), 5); }

// TEST(MathTest, Vec2Distance2) { EXPECT_EQ(scenic::Distance2({-1, -2}, {3, 4}), 52); }

TEST(MathTest, Vec2Sub) {
  EXPECT_TRUE(fidl::Equals(vec2({86, 75}) - vec2({30, -9}), vec2({56, 84})));
}

TEST(MathTest, Vec2AddAssign) {
  vec2 v{4, 2};
  // For assignment operators, also make sure the return is a self-ref.
  (v += {-5, 4}).x += .1f;
  EXPECT_TRUE(fidl::Equals(v, vec2({-.9f, 6})));
}

TEST(MathTest, Vec2ScalarMul) { EXPECT_TRUE(fidl::Equals(vec2({-3, .5f}) * 3, vec2({-9, 1.5f}))); }

TEST(MathTest, Vec2ScalarDivAssign) {
  vec2 v{5, -1};
  (v /= 2).x += .1f;
  EXPECT_TRUE(fidl::Equals(v, vec2({2.6f, -.5f})));
}

TEST(MathTest, Vec2ScalarDiv) {
  EXPECT_TRUE(fidl::Equals(vec2({-3, .5f}) / 2, vec2({-1.5f, .25f})));
}

}  // namespace scenic
