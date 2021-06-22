// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/join_strings.h"

#include <array>
#include <string_view>

#include <gtest/gtest.h>

namespace fxl {
namespace {

TEST(StringUtil, JoinStrings) {
  // Basic test.
  std::vector<std::string> v1 = {"foo", "bar", "baz"};
  EXPECT_EQ("foo,bar,baz", JoinStrings(v1, ","));
  EXPECT_EQ("foo, bar, baz", JoinStrings(v1, ", "));
  EXPECT_EQ("foobarbaz", JoinStrings(v1));

  // Check that empty strings are joined correctly.
  std::vector<std::string> v2 = {"foo", "", "bar", ""};
  EXPECT_EQ("foo,,bar,", JoinStrings(v2, ","));
  EXPECT_EQ("foo, , bar, ", JoinStrings(v2, ", "));
  EXPECT_EQ("foobar", JoinStrings(v2));

  // Check that only one string is joined correctly.
  std::vector<std::string> v3 = {"foo"};
  EXPECT_EQ("foo", JoinStrings(v3, ","));
  EXPECT_EQ("foo", JoinStrings(v3, ", "));
  EXPECT_EQ("foo", JoinStrings(v3));

  // Check that no string is joined correctly.
  std::vector<std::string> v4 = {};
  EXPECT_EQ("", JoinStrings(v4, ","));
  EXPECT_EQ("", JoinStrings(v4, ", "));
  EXPECT_EQ("", JoinStrings(v4));

  // Check that std::array can be used.
  std::array<std::string, 2> a = {{"foo", "bar"}};
  EXPECT_EQ("foo:bar", JoinStrings(a, ":"));

  // Check that std::string_view can be used.
  std::vector<std::string_view> sv;
  sv.push_back("foo");
  sv.push_back("bar");
  EXPECT_EQ("foo:bar", JoinStrings(sv, ":"));
}

}  // namespace
}  // namespace fxl
