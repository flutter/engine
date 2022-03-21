// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/viewport_size_holder.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(ViewportSizeHolderTest, UpdateViewportSize) {
  ViewportSizeHolder holder;
  ViewportSize old_first_size = {100.0, 200.0};
  holder.UpdateViewportSize(0, old_first_size);
  ViewportSize old_second_size = {500.0, 1000.0};
  holder.UpdateViewportSize(1, old_second_size);
  EXPECT_EQ(holder.GetViewportSize(0), old_first_size);
  EXPECT_EQ(holder.GetViewportSize(1), old_second_size);

  ViewportSize new_first_size = {200.0, 400.0};
  holder.UpdateViewportSize(0, new_first_size);
  ViewportSize new_second_size = {1000.0, 2000.0};
  holder.UpdateViewportSize(1, new_second_size);
  EXPECT_EQ(holder.GetViewportSize(0), new_first_size);
  EXPECT_EQ(holder.GetViewportSize(1), new_second_size);
}

TEST(ViewportSizeHolderTest, RemoveViewportSize) {
  ViewportSizeHolder holder;
  ViewportSize empty = {0.0, 0.0};
  EXPECT_EQ(holder.GetViewportSize(0), empty);

  ViewportSize size = {100.0, 200.0};
  holder.UpdateViewportSize(0, size);
  EXPECT_EQ(holder.GetViewportSize(0), size);

  holder.RemoveViewportSize(0);
  EXPECT_EQ(holder.GetViewportSize(0), empty);
}

TEST(ViewportSizeHolderTest, GetMaxViewportSize) {
  ViewportSizeHolder holder;
  ViewportSize first_size = {100.0, 200.0};
  holder.UpdateViewportSize(0, first_size);
  ViewportSize second_size = {200.0, 300.0};
  holder.UpdateViewportSize(1, second_size);
  EXPECT_EQ(holder.GetMaxViewportSize(), second_size);

  ViewportSize third_size = {300.0, 400.0};
  holder.UpdateViewportSize(2, third_size);
  EXPECT_EQ(holder.GetMaxViewportSize(), third_size);

  holder.RemoveViewportSize(2);
  EXPECT_EQ(holder.GetMaxViewportSize(), second_size);
  holder.RemoveViewportSize(1);
  EXPECT_EQ(holder.GetMaxViewportSize(), first_size);
}

}  // namespace testing
}  // namespace flutter
