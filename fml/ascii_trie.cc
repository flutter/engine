// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "flutter/fml/ascii_trie.h"
#include "flutter/fml/logging.h"

namespace fml {
// The max Ascii value.
static const int kMaxAsciiValue = 128;

struct AsciiTrie::TrieNode {
  AsciiTrie::TrieNode* children[kMaxAsciiValue];
};

typedef AsciiTrie::TrieNode TrieNode;

namespace {
void Add(TrieNode** trie, const char* entry) {
  int ch = entry[0];
  FML_DCHECK(ch < kMaxAsciiValue);
  if (ch != 0) {
    if (!*trie) {
      TrieNode* newNode = static_cast<TrieNode*>(calloc(sizeof(TrieNode), 1));
      *trie = newNode;
    }
    Add(&(*trie)->children[ch], entry + 1);
  }
}

TrieNode* MakeTrie(const std::vector<std::string>& entries) {
  TrieNode* result = nullptr;
  for (const std::string& entry : entries) {
    Add(&result, entry.c_str());
  }
  return result;
}

void FreeNode(TrieNode* node) {
  for (int i = 0; i < kMaxAsciiValue; ++i) {
    TrieNode* child = node->children[i];
    if (child) {
      FreeNode(child);
    }
  }
  free(node);
}
}  // namespace

AsciiTrie::AsciiTrie() : node_(nullptr, FreeNode) {}

void AsciiTrie::Fill(const std::vector<std::string>& entries) {
  node_.reset(MakeTrie(entries));
}

bool AsciiTrie::Query(TrieNode* trie, const char* query) {
  FML_DCHECK(trie);
  const char* char_position = query;
  TrieNode* trie_position = trie;
  TrieNode* child;
  int ch;
  while ((ch = *char_position) && (child = trie_position->children[ch])) {
    char_position++;
    trie_position = child;
  }
  return !child && trie_position != trie;
}
}  // namespace fml
