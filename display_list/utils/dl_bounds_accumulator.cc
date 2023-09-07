// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/utils/dl_bounds_accumulator.h"

namespace flutter {

void RectBoundsAccumulator::save() {
  saved_rects_.emplace_back(rect_);
  rect_ = AccumulationRect();
}
void RectBoundsAccumulator::restore() {
  if (!saved_rects_.empty()) {
    DlFRect layer_bounds = rect_.bounds();
    pop_and_accumulate(layer_bounds, nullptr);
  }
}
bool RectBoundsAccumulator::restore(
    std::function<bool(const DlFRect&, DlFRect&)> mapper,
    const DlFRect* clip) {
  bool success = true;
  if (!saved_rects_.empty()) {
    DlFRect layer_bounds = rect_.bounds();
    success = mapper(layer_bounds, layer_bounds);
    pop_and_accumulate(layer_bounds, clip);
  }
  return success;
}
void RectBoundsAccumulator::pop_and_accumulate(DlFRect& layer_bounds,
                                               const DlFRect* clip) {
  FML_DCHECK(!saved_rects_.empty());

  rect_ = saved_rects_.back();
  saved_rects_.pop_back();

  if (clip == nullptr) {
    accumulate(layer_bounds, -1);
  } else {
    auto clipped = layer_bounds.Intersection(*clip);
    if (clipped.has_value()) {
      accumulate(clipped.value(), -1);
    }
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
void RectBoundsAccumulator::AccumulationRect::accumulate(DlScalar left,
                                                         DlScalar top,
                                                         DlScalar right,
                                                         DlScalar bottom) {
  if (left < right && top < bottom) {
    if (min_x_ > left) {
      min_x_ = left;
    }
    if (min_y_ > top) {
      min_y_ = top;
    }
    if (max_x_ < right) {
      max_x_ = right;
    }
    if (max_y_ < bottom) {
      max_y_ = bottom;
    }
  }
}
DlFRect RectBoundsAccumulator::AccumulationRect::bounds() const {
  return (max_x_ >= min_x_ && max_y_ >= min_y_)
             ? DlFRect::MakeLTRB(min_x_, min_y_, max_x_, max_y_)
             : DlFRect();
}

void RTreeBoundsAccumulator::accumulate(const DlFRect& r, int index) {
  if (!r.IsEmpty()) {
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
    std::function<bool(const DlFRect& original, DlFRect& modified)> map,
    const DlFRect* clip) {
  if (saved_offsets_.empty()) {
    return true;
  }

  size_t previous_size = saved_offsets_.back();
  saved_offsets_.pop_back();

  bool success = true;
  for (size_t i = previous_size; i < rects_.size(); i++) {
    DlFRect original = rects_[i];
    if (!map(original, original)) {
      success = false;
    }
    if (clip != nullptr) {
      auto clipped = original.Intersection(*clip);
      if (!clipped.has_value()) {
        continue;
      }
      original = clipped.value();
    }
    rect_indices_[previous_size] = rect_indices_[i];
    rects_[previous_size] = original;
    previous_size++;
  }
  rects_.resize(previous_size);
  rect_indices_.resize(previous_size);
  return success;
}

DlFRect RTreeBoundsAccumulator::bounds() const {
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
