// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backtrace.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "logging.h"

namespace fml {
namespace testing {

TEST(BacktraceTest, CanGatherBacktrace) {
  if (!IsCrashHandlingSupported()) {
    GTEST_SKIP();
    return;
  }
  {
    auto trace = BacktraceHere(0);
    ASSERT_GT(trace.size(), 0u);
    ASSERT_NE(trace.find("Frame 0"), std::string::npos);
  }

  {
    auto trace = BacktraceHere(1);
    ASSERT_GT(trace.size(), 0u);
    ASSERT_NE(trace.find("Frame 0"), std::string::npos);
  }

  {
    auto trace = BacktraceHere(2);
    ASSERT_GT(trace.size(), 0u);
    ASSERT_NE(trace.find("Frame 0"), std::string::npos);
  }
}

TEST(BacktraceTest, BacktraceStartsWithCallerFunction) {
  if (!IsCrashHandlingSupported()) {
    GTEST_SKIP();
    return;
  }

  auto trace = BacktraceHere(0);
  ASSERT_GT(trace.size(), 0u);

  auto first_line_end = trace.find("\n");
  ASSERT_NE(first_line_end, std::string::npos);

  auto first_line = trace.substr(0, first_line_end);

  if (first_line.find("Unknown") != std::string::npos) {
    GTEST_SKIP() << "The symbols has been stripped from the executable.";
    return;
  }

  EXPECT_THAT(first_line, ::testing::HasSubstr("Frame 0"));
  EXPECT_THAT(first_line, ::testing::HasSubstr(__FUNCTION__));
}

}  // namespace testing
}  // namespace fml
