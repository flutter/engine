// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Fork of webrtc function_view_unittest:
// https://github.com/webrtc-uwp/webrtc/blob/master/api/function_view_unittest.cc

#include <memory>
#include <utility>

#include <functional>
#include "function_ref.h"
#include "gtest/gtest.h"

namespace fml {

namespace {

void exec(FunctionRef<void(void)> func) {
  func();
}

TEST(FunctionRefTest, Simple) {
  int x = 0;
  std::function<void(void)> func = [&] { x++; };
  exec(func);
  EXPECT_EQ(1, x);
}

}  // namespace
}  // namespace fml
