// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/substitute.h"

#include <gtest/gtest.h>

namespace fxl {
namespace {

TEST(Substitute, Args) {
  EXPECT_EQ("foo one bar", Substitute("foo $0 bar", "one"));
  EXPECT_EQ("foo one bar two foo", Substitute("foo $0 bar $1 foo", "one", "two"));
  EXPECT_EQ("foo one bar two foo three bar",
            Substitute("foo $0 bar $1 foo $2 bar", "one", "two", "three"));
  EXPECT_EQ("foo one bar two foo three bar four foo",
            Substitute("foo $0 bar $1 foo $2 bar $3 foo", "one", "two", "three", "four"));
  EXPECT_EQ(
      "foo one bar two foo three bar four foo five bar",
      Substitute("foo $0 bar $1 foo $2 bar $3 foo $4 bar", "one", "two", "three", "four", "five"));
  EXPECT_EQ("foo one bar two foo three bar four foo five bar six foo",
            Substitute("foo $0 bar $1 foo $2 bar $3 foo $4 bar $5 foo", "one", "two", "three",
                       "four", "five", "six"));
  EXPECT_EQ("foo one bar two foo three bar four foo five bar six foo seven bar",
            Substitute("foo $0 bar $1 foo $2 bar $3 foo $4 bar $5 foo $6 bar", "one", "two",
                       "three", "four", "five", "six", "seven"));
  EXPECT_EQ(
      "foo one bar two foo three bar four foo five bar six foo seven bar "
      "eight foo",
      Substitute("foo $0 bar $1 foo $2 bar $3 foo $4 bar $5 foo $6 bar "
                 "$7 foo",
                 "one", "two", "three", "four", "five", "six", "seven", "eight"));
  EXPECT_EQ(
      "foo one bar two foo three bar four foo five bar six foo seven bar "
      "eight foo nine bar",
      Substitute("foo $0 bar $1 foo $2 bar $3 foo $4 bar $5 foo $6 bar "
                 "$7 foo $8 bar",
                 "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"));
  EXPECT_EQ(
      "foo one bar two foo three bar four foo five bar six foo seven bar "
      "eight foo nine bar ten foo",
      Substitute("foo $0 bar $1 foo $2 bar $3 foo $4 bar $5 foo $6 bar "
                 "$7 foo $8 bar $9 foo",
                 "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"));
}

TEST(Substitute, ReusePlaceholder) {
  EXPECT_EQ("hello world hello world", Substitute("$0 $1 $0 $1", "hello", "world"));
}

TEST(Substitute, Numbers) { EXPECT_EQ("12345678910", Substitute("123$0567$19$2", "4", "8", "10")); }

TEST(Substitute, SingleDigitsOnly) {
  EXPECT_EQ("Hello world0", Substitute("$0 $10", "Hello", "world"));
}

TEST(Substitute, Packed) { EXPECT_EQ("foobar", Substitute("$0$1", "foo", "bar")); }

TEST(Substitute, Dollar) {
  EXPECT_EQ("$$$$", Substitute("$$$$$0$$", "$"));
  EXPECT_EQ("$hello$1$", Substitute("$$$0$$1$$", "hello"));
  EXPECT_EQ("$Dollar$literals$with multiple $ arguments",
            Substitute("$$Dollar$$$0$with $1 $$ $2", "literals", "multiple", "arguments"));
}

#ifdef NDEBUG
TEST(Substitute, Error) {
  // Not enough args.
  EXPECT_EQ("", Substitute("Hello $0$1", "world"));
  EXPECT_EQ("", Substitute("$0 world$1$2", "Hello", "!"));
  // Trailing '$'.
  EXPECT_EQ("", Substitute("$0 world$", "Hello"));
}
#else
TEST(Substitute, Error) {
  // Not enough args.
  EXPECT_DEATH_IF_SUPPORTED(Substitute("Hello $0$1", "world"),
                            "fxl::Substitute missing argument for \\$1:");
  EXPECT_DEATH_IF_SUPPORTED(Substitute("$0 world$1$2", "Hello", "!"),
                            "fxl::Substitute missing argument for \\$2:");
  EXPECT_DEATH_IF_SUPPORTED(Substitute("$0 world$", "Hello"),
                            "fxl::Substitute encountered trailing '\\$':");
}
#endif

}  // namespace
}  // namespace fxl
