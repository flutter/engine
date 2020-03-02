// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/flutter_rtree.h"
#include "flutter/fml/logging.h"

namespace flutter {

FlutterRTree::FlutterRTree() : fCount(0) {}

void FlutterRTree::insert(const SkRect boundsArray[],
                          const SkBBoxHierarchy::Metadata metadata[],
                          int N) {
  SkASSERT(0 == fCount);

  std::vector<Branch> branches;
  branches.reserve(N);

  for (int i = 0; i < N; i++) {
    const SkRect& bounds = boundsArray[i];
    if (bounds.isEmpty()) {
      continue;
    }
    Branch b;
    b.fBounds = bounds;
    b.fOpIndex = i;
    b.isDraw = (metadata == nullptr) ? false : metadata[i].isDraw;
    branches.push_back(b);
  }

  fCount = (int)branches.size();
  if (fCount) {
    if (1 == fCount) {
      fNodes.reserve(1);
      Node* n = this->allocateNodeAtLevel(0);
      n->fNumChildren = 1;
      n->fChildren[0] = branches[0];
      fRoot.fSubtree = n;
      fRoot.fBounds = branches[0].fBounds;
    } else {
      fNodes.reserve(CountNodes(fCount));
      fRoot = this->bulkLoad(&branches);
    }
  }
}

void FlutterRTree::insert(const SkRect boundsArray[], int N) {
  // TODO(egarciad): Ask Skia about removing this method signature.
  insert(boundsArray, nullptr, N);
}

FlutterRTree::Node* FlutterRTree::allocateNodeAtLevel(uint16_t level) {
  SkDEBUGCODE(Node* p = fNodes.data());
  fNodes.push_back(Node{});
  Node& out = fNodes.back();
  SkASSERT(fNodes.data() == p);  // If this fails, we didn't reserve() enough.
  out.fNumChildren = 0;
  out.fLevel = level;
  return &out;
}

// This function parallels bulkLoad, but just counts how many nodes bulkLoad
// would allocate.
int FlutterRTree::CountNodes(int branches) {
  if (branches == 1) {
    return 1;
  }
  int numBranches = branches / kMaxChildren;
  int remainder = branches % kMaxChildren;
  if (remainder > 0) {
    numBranches++;
    if (remainder >= kMinChildren) {
      remainder = 0;
    } else {
      remainder = kMinChildren - remainder;
    }
  }
  int currentBranch = 0;
  int nodes = 0;
  while (currentBranch < branches) {
    int incrementBy = kMaxChildren;
    if (remainder != 0) {
      if (remainder <= kMaxChildren - kMinChildren) {
        incrementBy -= remainder;
        remainder = 0;
      } else {
        incrementBy = kMinChildren;
        remainder -= kMaxChildren - kMinChildren;
      }
    }
    nodes++;
    currentBranch++;
    for (int k = 1; k < incrementBy && currentBranch < branches; ++k) {
      currentBranch++;
    }
  }
  return nodes + CountNodes(nodes);
}

FlutterRTree::Branch FlutterRTree::bulkLoad(std::vector<Branch>* branches,
                                            int level) {
  if (branches->size() == 1) {  // Only one branch.  It will be the root.
    return (*branches)[0];
  }

  // We might sort our branches here, but we expect Blink gives us a reasonable
  // x,y order. Skipping a call to sort (in Y) here resulted in a 17% win for
  // recording with negligible difference in playback speed.
  int numBranches = (int)branches->size() / kMaxChildren;
  int remainder = (int)branches->size() % kMaxChildren;
  int newBranches = 0;

  if (remainder > 0) {
    ++numBranches;
    // If the remainder isn't enough to fill a node, we'll add fewer nodes to
    // other branches.
    if (remainder >= kMinChildren) {
      remainder = 0;
    } else {
      remainder = kMinChildren - remainder;
    }
  }

  int currentBranch = 0;
  while (currentBranch < (int)branches->size()) {
    int incrementBy = kMaxChildren;
    if (remainder != 0) {
      // if need be, omit some nodes to make up for remainder
      if (remainder <= kMaxChildren - kMinChildren) {
        incrementBy -= remainder;
        remainder = 0;
      } else {
        incrementBy = kMinChildren;
        remainder -= kMaxChildren - kMinChildren;
      }
    }
    Node* n = allocateNodeAtLevel(level);
    n->fNumChildren = 1;
    n->fChildren[0] = (*branches)[currentBranch];
    Branch b;
    b.fBounds = (*branches)[currentBranch].fBounds;
    b.fSubtree = n;
    ++currentBranch;
    for (int k = 1; k < incrementBy && currentBranch < (int)branches->size();
         ++k) {
      b.fBounds.join((*branches)[currentBranch].fBounds);
      n->fChildren[k] = (*branches)[currentBranch];
      ++n->fNumChildren;
      ++currentBranch;
    }
    (*branches)[newBranches] = b;
    ++newBranches;
  }
  branches->resize(newBranches);
  return this->bulkLoad(branches, level + 1);
}

void FlutterRTree::search(const SkRect& query,
                          std::vector<int>* results) const {
  if (fCount > 0 && SkRect::Intersects(fRoot.fBounds, query)) {
    this->search(fRoot.fSubtree, query, results);
  }
}

void FlutterRTree::search(Node* node,
                          const SkRect& query,
                          std::vector<int>* results) const {
  for (int i = 0; i < node->fNumChildren; ++i) {
    if (!SkRect::Intersects(node->fChildren[i].fBounds, query)) {
      continue;
    }
    if (0 != node->fLevel) {
      this->search(node->fChildren[i].fSubtree, query, results);
      continue;
    }
    results->push_back(node->fChildren[i].fOpIndex);
  }
}

void FlutterRTree::searchRects(const SkRect& query,
                               std::vector<SkRect*>* results) const {
  if (fCount > 0 && SkRect::Intersects(fRoot.fBounds, query)) {
    this->searchRects(fRoot.fSubtree, query, results);
  }
}

void FlutterRTree::searchRects(Node* node,
                               const SkRect& query,
                               std::vector<SkRect*>* results) const {
  if (!SkRect::Intersects(fRoot.fBounds, query)) {
    return;
  }
  for (int i = 0; i < node->fNumChildren; ++i) {
    if (!SkRect::Intersects(node->fChildren[i].fBounds, query)) {
      continue;
    }
    // Non-leaf node
    if (0 != node->fLevel) {
      this->searchRects(node->fChildren[i].fSubtree, query, results);
      continue;
    }
    // Ignore records that don't draw anything.
    // TODO(egarciad): Figure out if this can be moved to insert().
    if (!node->fChildren[i].isDraw) {
      continue;
    }
    SkRect* currentRecordRect = &node->fChildren[i].fBounds;
    bool replacedExistingRect = false;

    std::vector<SkRect*> currentResults = *results;
    // If the current record rect intersects with any of the rects in the
    // result, then join them, and update the rect in results.
    for (size_t j = 0; !replacedExistingRect && j < results->size(); j++) {
      if (SkRect::Intersects(*currentResults[j], *currentRecordRect)) {
        currentResults[j]->join(*currentRecordRect);
        replacedExistingRect = true;
      }
    }
    if (!replacedExistingRect) {
      results->push_back(currentRecordRect);
    }
  }
}

size_t FlutterRTree::bytesUsed() const {
  size_t byteCount = sizeof(FlutterRTree);
  byteCount += fNodes.capacity() * sizeof(Node);
  return byteCount;
}

FlutterRTreeFactory::FlutterRTreeFactory(sk_sp<FlutterRTree>& r_tree)
    : r_tree_(r_tree) {}

sk_sp<SkBBoxHierarchy> FlutterRTreeFactory::operator()() const {
  return r_tree_;
}

}  // namespace flutter
