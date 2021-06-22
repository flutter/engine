// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/ascii.h"

#include <gtest/gtest.h>

namespace fxl {
namespace {

TEST(StringUtil, ToLowerASCII) {
  EXPECT_EQ(',', fxl::ToLowerASCII(','));
  EXPECT_EQ('a', fxl::ToLowerASCII('a'));
  EXPECT_EQ('a', fxl::ToLowerASCII('A'));
}

TEST(StringUtil, ToUpperASCII) {
  EXPECT_EQ(',', fxl::ToUpperASCII(','));
  EXPECT_EQ('A', fxl::ToUpperASCII('a'));
  EXPECT_EQ('A', fxl::ToUpperASCII('A'));
}

TEST(StringUtil, EqualsCaseInsensitiveASCII) {
  EXPECT_TRUE(EqualsCaseInsensitiveASCII("", ""));
  EXPECT_TRUE(EqualsCaseInsensitiveASCII("abcd", "abcd"));
  EXPECT_TRUE(EqualsCaseInsensitiveASCII("abcd", "aBcD"));
  EXPECT_TRUE(EqualsCaseInsensitiveASCII("abcd", "ABCD"));

  EXPECT_FALSE(EqualsCaseInsensitiveASCII("abcd", ""));
  EXPECT_FALSE(EqualsCaseInsensitiveASCII("abcd", "abc"));
  EXPECT_FALSE(EqualsCaseInsensitiveASCII("abcd", "ABC"));
  EXPECT_FALSE(EqualsCaseInsensitiveASCII("abcd", "ABCDE"));
}

}  // namespace
}  // namespace fxl
