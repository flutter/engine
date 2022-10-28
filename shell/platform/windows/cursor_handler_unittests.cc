// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "flutter/shell/platform/windows/cursor_handler.h"

#include <memory>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {
TEST(CursorHandlerTest, CreateDummyCursor) {
  // create 4x4 rawRGBA dummy cursor arguments
  std::vector<uint8_t> buffer;
  for (int i = 0; i < 4 * 4 * 4; i++) {
    buffer.push_back(0);
  }
  auto cursor = GetCursorFromBuffer(std::move(buffer), 0, 0, 4, 4);
  EXPECT_NE(cursor, nullptr);
}

}  // namespace testing
}  // namespace flutter
