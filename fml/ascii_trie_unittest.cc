// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/ascii_trie.h"
#include "gtest/gtest.h"

using fml::AsciiTrie;

TEST(AsciiTableTest, Simple) {
  AsciiTrie trie;
  auto entries = std::vector<std::string> {"foo"};
  trie.fill(entries);
  ASSERT_TRUE(trie.query("foobar"));
  ASSERT_FALSE(trie.query("google"));
}

TEST(AsciiTableTest, ExactMatch) {
  AsciiTrie trie;
  auto entries = std::vector<std::string> {"foo"};
  trie.fill(entries);
  ASSERT_TRUE(trie.query("foo"));
}

TEST(AsciiTableTest, Empty) {
  AsciiTrie trie;
  ASSERT_TRUE(trie.query("foo"));
}

TEST(AsciiTableTest, MultipleEntries) {
  AsciiTrie trie;
  auto entries = std::vector<std::string> {"foo", "bar"};
  trie.fill(entries);
  ASSERT_TRUE(trie.query("foozzz"));
  ASSERT_TRUE(trie.query("barzzz"));
}
