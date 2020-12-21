// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "string_utils.h"

#include <errno.h>
#include <stddef.h>

#include "gtest/gtest.h"

namespace base {

TEST(StringUtilsTest, StringPrintfEmpty) {
  EXPECT_EQ("", base::StringPrintf("%s", ""));
}

TEST(StringUtilsTest, StringPrintfMisc) {
  EXPECT_EQ("123hello w", StringPrintf("%3d%2s %1c", 123, "hello", 'w'));
}
// Test that StringPrintf and StringAppendV do not change errno.
TEST(StringUtilsTest, StringPrintfErrno) {
  errno = 1;
  EXPECT_EQ("", StringPrintf("%s", ""));
  EXPECT_EQ(1, errno);
}

}  // namespace base
