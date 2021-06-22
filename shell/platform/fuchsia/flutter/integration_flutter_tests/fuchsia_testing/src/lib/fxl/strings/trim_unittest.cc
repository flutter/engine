// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/trim.h"

#include <gtest/gtest.h>

namespace fxl {
namespace {

TEST(StringUtil, TrimString) {
  std::string_view sw = " \tHello World\t ";

  EXPECT_EQ(sw, TrimString(sw, ""));
  EXPECT_EQ(sw, TrimString(sw, "abc"));

  EXPECT_EQ(std::string(sw.begin() + 1, sw.end() - 1), TrimString(sw, "abc "));

  EXPECT_EQ(std::string(sw.begin() + 2, sw.end() - 2), TrimString(sw, "abc \t"));

  EXPECT_EQ(std::string(), TrimString(sw, "HWorlde \t"));
}

}  // namespace
}  // namespace fxl
