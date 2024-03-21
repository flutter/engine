// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/utils/dl_bounds_accumulator.h"

namespace flutter {

void RectBoundsAccumulator::accumulate(const DlRect& r, int index) {
  if (r.fLeft < r.fRight && r.fTop < r.fBottom) {
    rect_.accumulate(r.fLeft, r.fTop);
    rect_.accumulate(r.fRight, r.fBottom);
  }
}

void RectBoundsAccumulator::save() {
  saved_rects_.emplace_back(rect_);
  rect_ = AccumulationRect();
}
void RectBoundsAccumulator::restore() {
  if (!saved_rects_.empty()) {
    DlRect layer_bounds = rect_.bounds();
    pop_and_accumulate(layer_bounds, nullptr);
  }
}
bool RectBoundsAccumulator::restore(
    std::function<bool(const DlRect&, DlRect&)> mapper,
    const DlRect* clip) {
  bool success = true;
  if (!saved_rects_.empty()) {
    DlRect layer_bounds = rect_.bounds();
    success = mapper(layer_bounds, layer_bounds);
    pop_and_accumulate(layer_bounds, clip);
  }
  return success;
}
void RectBoundsAccumulator::pop_and_accumulate(DlRect& layer_bounds,
                                               const DlRect* clip) {
  FML_DCHECK(!saved_rects_.empty());

  rect_ = saved_rects_.back();
  saved_rects_.pop_back();

  if (clip == nullptr || layer_bounds.intersect(*clip)) {
    accumulate(layer_bounds, -1);
  }
}

RectBoundsAccumulator::AccumulationRect::AccumulationRect() {
  min_x_ = std::numeric_limits<DlScalar>::infinity();
  min_y_ = std::numeric_limits<DlScalar>::infinity();
  max_x_ = -std::numeric_limits<DlScalar>::infinity();
  max_y_ = -std::numeric_limits<DlScalar>::infinity();
}
void RectBoundsAccumulator::AccumulationRect::accumulate(DlScalar x,
                                                         DlScalar y) {
  if (min_x_ > x) {
    min_x_ = x;
  }
  if (min_y_ > y) {
    min_y_ = y;
  }
  if (max_x_ < x) {
    max_x_ = x;
  }
  if (max_y_ < y) {
    max_y_ = y;
  }
}
DlRect RectBoundsAccumulator::AccumulationRect::bounds() const {
  return (max_x_ >= min_x_ && max_y_ >= min_y_)
             ? DlRect::MakeLTRB(min_x_, min_y_, max_x_, max_y_)
             : DlRect::MakeEmpty();
}

void RTreeBoundsAccumulator::accumulate(const DlRect& r, int index) {
  if (r.fLeft < r.fRight && r.fTop < r.fBottom) {
    rects_.push_back(r);
    rect_indices_.push_back(index);
  }
}
void RTreeBoundsAccumulator::save() {
  saved_offsets_.push_back(rects_.size());
}
void RTreeBoundsAccumulator::restore() {
  if (saved_offsets_.empty()) {
    return;
  }

  saved_offsets_.pop_back();
}
bool RTreeBoundsAccumulator::restore(
    std::function<bool(const DlRect& original, DlRect& modified)> map,
    const DlRect* clip) {
  if (saved_offsets_.empty()) {
    return true;
  }

  size_t previous_size = saved_offsets_.back();
  saved_offsets_.pop_back();

  bool success = true;
  for (size_t i = previous_size; i < rects_.size(); i++) {
    DlRect original = rects_[i];
    if (!map(original, original)) {
      success = false;
    }
    if (clip == nullptr || original.intersect(*clip)) {
      rect_indices_[previous_size] = rect_indices_[i];
      rects_[previous_size] = original;
      previous_size++;
    }
  }
  rects_.resize(previous_size);
  rect_indices_.resize(previous_size);
  return success;
}

DlRect RTreeBoundsAccumulator::bounds() const {
  FML_DCHECK(saved_offsets_.empty());
  RectBoundsAccumulator accumulator;
  for (auto& rect : rects_) {
    accumulator.accumulate(rect, 0);
  }
  return accumulator.bounds();
}

sk_sp<DlRTree> RTreeBoundsAccumulator::rtree() const {
  FML_DCHECK(saved_offsets_.empty());
  return sk_make_sp<DlRTree>(rects_.data(), rects_.size(), rect_indices_.data(),
                             [](int id) { return id >= 0; });
}

}  // namespace flutter
