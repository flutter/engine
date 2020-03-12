// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_ASCIITRIE_H_
#define FLUTTER_FML_ASCIITRIE_H_

#include <string>
#include <vector>

namespace fml {

/// A trie for looking for ASCII prefixes.
class AsciiTrie {
 public:
  ~AsciiTrie();

  /// Clear and insert all the entries into the trie.
  void fill(const std::vector<std::string>& entries);

  /// Returns true if \p argument is prefixed by the contents of the trie.
  inline bool query(const char* argument) {
    return !node_ || query(node_, argument);
  }

  struct TrieNode {
    TrieNode* children[128];
  };

 private:
  static bool query(TrieNode* trie, const char* query);
  TrieNode* node_ = nullptr;
};
}  // namespace fml

#endif
