// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_RTREE_H_
#define FLUTTER_DISPLAY_LIST_RTREE_H_

#include <list>

#include "flutter/fml/logging.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkRefCnt.h"

namespace flutter {

/// An R-Tree that stores a list of bounding rectangles with optional
/// associated IDs and returns either a vector of result indices which
/// can be used to get an ID or a bounds of the associated entry in the
/// same order that the rectangles and IDs were provided to the constructor.
class DlRTree : public SkRefCnt {
 private:
  static constexpr int kMaxChildren = 11;

  // Leaf nodes at start of vector have an ID,
  // Internal nodes after that have child index and count.
  struct Node {
    SkRect bounds;
    union {
      struct {
        uint32_t index;
        uint32_t count;
      } child;
      int id;
    };
  };

 public:
  /// Construct an R-Tree from the list of rectangles respecting the
  /// order in which they appear in the list. An optional array of
  /// IDs can be provided to tag each rectangle with information needed
  /// by the caller.
  DlRTree(
      const SkRect rects[],
      int unfilteredN,
      const int ids[] = nullptr,
      bool p(int) = [](int) { return true; });

  /// Search the rectangles and return a vector of result indices for
  /// rectangles that intersect the query. Note that the indices are
  /// internal indices of the stored data and not the index of the
  /// rectangles or ids in the constructor. The indices will be stored
  /// in the results vector according to the order in which the rectangles
  /// were supplied in the constructor, even though the actual numeric
  /// values may not match.
  void search(const SkRect& query, std::vector<int>* results) const {
    if (query.isEmpty()) {
      return;
    }
    if (nodes_.size() <= 0) {
      return;
    }
    const Node& root = nodes_.back();
    if (root.bounds.intersects(query)) {
      if (nodes_.size() == 1) {
        // The root node is the only node and it is a leaf node
        results->push_back(0);
      } else {
        search(root, query, results);
      }
    }
  }

  /// Return the ID for the indicated result of a query
  int id(int result_index) const {
    FML_DCHECK(result_index < leaf_count_);
    return nodes_[result_index].id;
  }

  /// Return the rectangle bounds for the indicated result of a query
  const SkRect& bounds(int result_index) const {
    FML_DCHECK(result_index < leaf_count_);
    return nodes_[result_index].bounds;
  }

  size_t bytesUsed() const {
    return sizeof(DlRTree) + sizeof(Node) * nodes_.size();
  }

  // Finds the rects in the tree that represent drawing operations and intersect
  // with the query rect.
  //
  // When two rects intersect with each other, they are joined into a single
  // rect which also intersects with the query rect. In other words, the bounds
  // of each rect in the result list are mutually exclusive.
  std::list<SkRect> searchNonOverlappingDrawnRects(const SkRect& query) const;

  // Insertion count (not overall node count, which may be greater).
  // int getCount() const { return all_ops_count_; }

 private:
  void search(const Node& parent,
              const SkRect& query,
              std::vector<int>* results) const;

  std::vector<Node> nodes_;
  int leaf_count_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_RTREE_H_
