// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "flutter/fml/ascii_trie.h"

namespace fml {
typedef AsciiTrie::TrieNode TrieNode;

namespace {
void Add(TrieNode** trie, const char* entry) {
  int ch = entry[0];
  assert(ch < 128);
  if (ch != 0) {
    if (!*trie) {
      TrieNode* newNode = (TrieNode*)calloc(sizeof(TrieNode), 1);
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
  for (int i = 0; i < 128; ++i) {
    if (node->children[i]) {
      TrieNode* child = node->children[i];
      if (child) {
        FreeNode(child);
      }
    }
  }
  free(node);
}
}  // namespace

AsciiTrie::~AsciiTrie() {
  if (node_) {
    FreeNode(node_);
  }
}

void AsciiTrie::fill(const std::vector<std::string>& entries) {
  if (node_) {
    FreeNode(node_);
  }
  node_ = MakeTrie(entries);
}

bool AsciiTrie::query(TrieNode* trie, const char* query) {
  assert(trie);
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
