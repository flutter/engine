// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_rtree.h"

#include "flutter/fml/logging.h"

namespace flutter {

DlRTree::DlRTree(const SkRect rects[],
                 int N,
                 const int ids[],
                 bool p(int),
                 int invalid_id)
    : leaf_count_(0), invalid_id_(invalid_id) {
  // Count the number of rectangles we actually want to track,
  // which includes only non-empty rectangles whose optional
  // ID is not filtered by the predicate.
  for (int i = 0; i < N; i++) {
    if (!rects[i].isEmpty()) {
      if (ids == nullptr || p(ids[i])) {
        leaf_count_++;
      }
    }
  }

  // Count the total number of nodes (leaf and internal) up front
  // we we can resize the vector once.
  int total = leaf_count_;
  int n_nodes = leaf_count_;
  while (n_nodes > 1) {
    int n_groups = (n_nodes + kMaxChildren - 1) / kMaxChildren;
    total += n_groups;
    n_nodes = n_groups;
  }

  nodes_.resize(total);

  // Now place only the tracked rectangles into the nodes array
  // in the first leaf_count_ entries.
  int node_index = 0;
  int id = invalid_id;
  for (int i = 0; i < N; i++) {
    if (!rects[i].isEmpty()) {
      if (ids == nullptr || p(id = ids[i])) {
        Node& node = nodes_[node_index++];
        node.bounds = rects[i];
        node.id = id;
      }
    }
  }
  FML_DCHECK(node_index == leaf_count_);

  // Continually process the previous level of nodes, combining them
  // into groups of at most |kMaxChildren| sub-nodes and joining
  // their bounds into a parent node.
  // Each level will end up reduced by a factor of up to kMaxChildren
  // until there is just one node left, which is the root node of
  // the R-Tree.
  int level_start = 0;
  n_nodes = leaf_count_;
  while (n_nodes > 1) {
    int n_groups = (n_nodes + kMaxChildren - 1) / kMaxChildren;
    int node_count = kMaxChildren;
    int remainder = n_groups * kMaxChildren - n_nodes;
    if (node_count > kMaxChildren - remainder) {
      // TODO(flar): use a distributed remainder computation to
      // distribute the nodes more evenly
      node_count = kMaxChildren - remainder;
    }
    for (int i = 0; i < n_groups; i++) {
      Node& node = nodes_[node_index++];
      node.bounds.setEmpty();
      node.child.index = level_start;
      node.child.count = node_count;
      for (int j = 0; j < node_count; j++) {
        node.bounds.join(nodes_[level_start++].bounds);
      }
      node_count = kMaxChildren;
    }
    // level_start should be where node_index started this round
    FML_DCHECK(level_start + n_groups == node_index);
    n_nodes = n_groups;
  }
  FML_DCHECK(node_index == total);
}

void DlRTree::search(const SkRect& query, std::vector<int>* results) const {
  if (query.isEmpty()) {
    return;
  }
  if (nodes_.size() <= 0) {
    FML_DCHECK(leaf_count_ == 0);
    return;
  }
  const Node& root = nodes_.back();
  if (root.bounds.intersects(query)) {
    if (nodes_.size() == 1) {
      FML_DCHECK(leaf_count_ == 1);
      // The root node is the only node and it is a leaf node
      results->push_back(0);
    } else {
      search(root, query, results);
    }
  }
}

std::list<SkRect> DlRTree::searchNonOverlappingDrawnRects(
    const SkRect& query) const {
  // Get the indexes for the operations that intersect with the query rect.
  std::vector<int> intermediary_results;
  search(query, &intermediary_results);

  std::list<SkRect> final_results;
  for (int index : intermediary_results) {
    auto current_record_rect = bounds(index);
    auto replaced_existing_rect = false;
    // // If the current record rect intersects with any of the rects in the
    // // result list, then join them, and update the rect in final_results.
    std::list<SkRect>::iterator curr_rect_itr = final_results.begin();
    std::list<SkRect>::iterator first_intersecting_rect_itr;
    while (!replaced_existing_rect && curr_rect_itr != final_results.end()) {
      if (SkRect::Intersects(*curr_rect_itr, current_record_rect)) {
        replaced_existing_rect = true;
        first_intersecting_rect_itr = curr_rect_itr;
        curr_rect_itr->join(current_record_rect);
      }
      curr_rect_itr++;
    }
    // It's possible that the result contains duplicated rects at this point.
    // For example, consider a result list that contains rects A, B. If a
    // new rect C is a superset of A and B, then A and B are the same set after
    // the merge. As a result, find such cases and remove them from the result
    // list.
    while (replaced_existing_rect && curr_rect_itr != final_results.end()) {
      if (SkRect::Intersects(*curr_rect_itr, *first_intersecting_rect_itr)) {
        first_intersecting_rect_itr->join(*curr_rect_itr);
        curr_rect_itr = final_results.erase(curr_rect_itr);
      } else {
        curr_rect_itr++;
      }
    }
    if (!replaced_existing_rect) {
      final_results.push_back(current_record_rect);
    }
  }
  return final_results;
}

void DlRTree::search(const Node& parent,
                     const SkRect& query,
                     std::vector<int>* results) const {
  // Caller protects against empty query
  int start = parent.child.index;
  int end = start + parent.child.count;
  for (int i = start; i < end; i++) {
    const Node& node = nodes_[i];
    if (node.bounds.intersects(query)) {
      if (i < leaf_count_) {
        results->push_back(i);
      } else {
        search(node, query, results);
      }
    }
  }
}

}  // namespace flutter
