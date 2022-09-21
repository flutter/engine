// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backtrace.h"

#include <csignal>

#include "gtest/gtest.h"

namespace fml {
namespace testing {

TEST(BacktraceDeathTest, CanGatherBacktrace) {
  if (!IsCrashHandlingSupported()) {
    GTEST_SKIP();
    return;
  }
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH_IF_SUPPORTED({ std::abort(); }, "SIGABRT");
}

}  // namespace testing
}  // namespace fml
