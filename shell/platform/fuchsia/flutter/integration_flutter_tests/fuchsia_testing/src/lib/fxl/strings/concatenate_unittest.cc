// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/concatenate.h"

#include <gtest/gtest.h>

namespace fxl {
namespace {

// Creates correct std::strings from C-style string constants with \0 bytes
// inside.
std::string operator"" _s(const char* str, size_t size) { return std::string(str, size); }

TEST(StringUtil, Concatenate) {
  EXPECT_EQ("", Concatenate({}));
  EXPECT_EQ("a", Concatenate({"a"}));
  EXPECT_EQ("ab", Concatenate({"a", "b"}));

  std::string such = "such";
  EXPECT_EQ("wow such useful", Concatenate({"wow ", such, " useful"}));

  std::string with_zeroes = "abc\0def"_s;
  EXPECT_EQ("abc\0def\0ghi"_s, Concatenate({with_zeroes, "\0ghi"_s}));
}

}  // namespace
}  // namespace fxl
