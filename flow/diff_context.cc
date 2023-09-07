// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/diff_context.h"
#include "flutter/flow/layers/layer.h"

namespace flutter {

DiffContext::DiffContext(DlISize frame_size,
                         PaintRegionMap& this_frame_paint_region_map,
                         const PaintRegionMap& last_frame_paint_region_map,
                         bool has_raster_cache,
                         bool impeller_enabled)
    : clip_tracker_(DisplayListMatrixClipTracker(kMaxCullRect, DlTransform())),
      rects_(std::make_shared<std::vector<DlFRect>>()),
      frame_size_(frame_size),
      this_frame_paint_region_map_(this_frame_paint_region_map),
      last_frame_paint_region_map_(last_frame_paint_region_map),
      has_raster_cache_(has_raster_cache),
      impeller_enabled_(impeller_enabled) {}

void DiffContext::BeginSubtree() {
  state_stack_.push_back(state_);

  bool had_integral_transform = state_.integral_transform;
  state_.rect_index = rects_->size();
  state_.has_filter_bounds_adjustment = false;
  state_.has_texture = false;
  state_.integral_transform = false;

  state_.clip_tracker_save_count = clip_tracker_.getSaveCount();
  clip_tracker_.save();

  if (had_integral_transform) {
    MakeCurrentTransformIntegral();
  }
}

void DiffContext::EndSubtree() {
  FML_DCHECK(!state_stack_.empty());
  if (state_.has_filter_bounds_adjustment) {
    filter_bounds_adjustment_stack_.pop_back();
  }
  clip_tracker_.restoreToCount(state_.clip_tracker_save_count);
  state_ = state_stack_.back();
  state_stack_.pop_back();
}

DiffContext::State::State()
    : dirty(false),
      rect_index(0),
      integral_transform(false),
      clip_tracker_save_count(0),
      has_filter_bounds_adjustment(false),
      has_texture(false) {}

void DiffContext::PushTransform(const DlTransform& transform) {
  clip_tracker_.transform(transform);
}

void DiffContext::MakeCurrentTransformIntegral() {
  clip_tracker_.setIntegerTranslation();
}

void DiffContext::PushFilterBoundsAdjustment(
    const FilterBoundsAdjustment& filter) {
  FML_DCHECK(state_.has_filter_bounds_adjustment == false);
  state_.has_filter_bounds_adjustment = true;
  filter_bounds_adjustment_stack_.push_back(filter);
}

DlFRect DiffContext::ApplyFilterBoundsAdjustment(DlFRect rect) const {
  // Apply filter bounds adjustment in reverse order
  for (auto i = filter_bounds_adjustment_stack_.rbegin();
       i != filter_bounds_adjustment_stack_.rend(); ++i) {
    rect = (*i)(rect);
  }
  return rect;
}

void DiffContext::AlignRect(DlIRect& rect,
                            int horizontal_alignment,
                            int vertical_alignment) const {
  auto top = rect.top();
  auto left = rect.left();
  auto right = rect.right();
  auto bottom = rect.bottom();
  if (top % vertical_alignment != 0) {
    top -= top % vertical_alignment;
  }
  if (left % horizontal_alignment != 0) {
    left -= left % horizontal_alignment;
  }
  if (right % horizontal_alignment != 0) {
    right += horizontal_alignment - right % horizontal_alignment;
  }
  if (bottom % vertical_alignment != 0) {
    bottom += vertical_alignment - bottom % vertical_alignment;
  }
  right = std::min(right, (int)frame_size_.width());
  bottom = std::min(bottom, (int)frame_size_.height());
  rect = DlIRect::MakeLTRB(left, top, right, bottom);
}

Damage DiffContext::ComputeDamage(const DlIRect& accumulated_buffer_damage,
                                  int horizontal_clip_alignment,
                                  int vertical_clip_alignment) const {
  DlFRect buffer_damage =
      DlFRect::MakeBounds(accumulated_buffer_damage).Union(damage_);
  DlFRect frame_damage(damage_);

  for (const auto& r : readbacks_) {
    DlFRect rect = DlFRect::MakeBounds(r.rect);
    if (rect.Intersects(frame_damage)) {
      frame_damage = frame_damage.Union(rect);
    }
    if (rect.Intersects(buffer_damage)) {
      buffer_damage = buffer_damage.Union(rect);
    }
  }

  Damage res = {
      .frame_damage = DlIRect::MakeRoundedOut(frame_damage),
      .buffer_damage = DlIRect::MakeRoundedOut(buffer_damage),
  };

  DlIRect frame_clip = DlIRect::MakeSize(frame_size_);
  res.buffer_damage = res.buffer_damage.IntersectionOrEmpty(frame_clip);
  res.frame_damage = res.frame_damage.IntersectionOrEmpty(frame_clip);

  if (horizontal_clip_alignment > 1 || vertical_clip_alignment > 1) {
    AlignRect(res.buffer_damage, horizontal_clip_alignment,
              vertical_clip_alignment);
    AlignRect(res.frame_damage, horizontal_clip_alignment,
              vertical_clip_alignment);
  }
  return res;
}

DlFRect DiffContext::MapRect(const DlFRect& rect) {
  DlFRect mapped_rect(rect);
  clip_tracker_.mapRect(&mapped_rect);
  return mapped_rect;
}

bool DiffContext::PushCullRect(const DlFRect& clip) {
  clip_tracker_.clipRect(clip, DlCanvas::ClipOp::kIntersect, false);
  return !clip_tracker_.device_cull_rect().IsEmpty();
}

DlTransform DiffContext::GetTransform() const {
  return clip_tracker_.matrix();
}

DlFRect DiffContext::GetCullRect() const {
  return clip_tracker_.local_cull_rect();
}

void DiffContext::MarkSubtreeDirty(const PaintRegion& previous_paint_region) {
  FML_DCHECK(!IsSubtreeDirty());
  if (previous_paint_region.is_valid()) {
    AddDamage(previous_paint_region);
  }
  state_.dirty = true;
}

void DiffContext::MarkSubtreeDirty(const DlFRect& previous_paint_region) {
  FML_DCHECK(!IsSubtreeDirty());
  AddDamage(previous_paint_region);
  state_.dirty = true;
}

void DiffContext::AddLayerBounds(const DlFRect& rect) {
  // During painting we cull based on non-overriden transform and then
  // override the transform right before paint. Do the same thing here to get
  // identical paint rect.
  auto transformed_rect = ApplyFilterBoundsAdjustment(MapRect(rect));
  if (transformed_rect.Intersects(clip_tracker_.device_cull_rect())) {
    if (state_.integral_transform) {
      clip_tracker_.save();
      MakeCurrentTransformIntegral();
      transformed_rect = ApplyFilterBoundsAdjustment(MapRect(rect));
      clip_tracker_.restore();
    }
    rects_->push_back(transformed_rect);
    if (IsSubtreeDirty()) {
      AddDamage(transformed_rect);
    }
  }
}

void DiffContext::MarkSubtreeHasTextureLayer() {
  // Set the has_texture flag on current state and all parent states. That
  // way we'll know that we can't skip diff for retained layers because
  // they contain a TextureLayer.
  for (auto& state : state_stack_) {
    state.has_texture = true;
  }
  state_.has_texture = true;
}

void DiffContext::AddExistingPaintRegion(const PaintRegion& region) {
  // Adding paint region for retained layer implies that current subtree is not
  // dirty, so we know, for example, that the inherited transforms must match
  FML_DCHECK(!IsSubtreeDirty());
  if (region.is_valid()) {
    rects_->insert(rects_->end(), region.begin(), region.end());
  }
}

void DiffContext::AddReadbackRegion(const DlIRect& rect) {
  Readback readback;
  readback.rect = rect;
  readback.position = rects_->size();
  // Push empty rect as a placeholder for position in current subtree
  rects_->emplace_back();
  readbacks_.push_back(readback);
}

PaintRegion DiffContext::CurrentSubtreeRegion() const {
  bool has_readback = std::any_of(
      readbacks_.begin(), readbacks_.end(),
      [&](const Readback& r) { return r.position >= state_.rect_index; });
  return PaintRegion(rects_, state_.rect_index, rects_->size(), has_readback,
                     state_.has_texture);
}

void DiffContext::AddDamage(const PaintRegion& damage) {
  FML_DCHECK(damage.is_valid());
  for (const auto& r : damage) {
    damage_ = damage_.Union(r);
  }
}

void DiffContext::AddDamage(const DlFRect& rect) {
  damage_ = damage_.Union(rect);
}

void DiffContext::SetLayerPaintRegion(const Layer* layer,
                                      const PaintRegion& region) {
  this_frame_paint_region_map_[layer->unique_id()] = region;
}

PaintRegion DiffContext::GetOldLayerPaintRegion(const Layer* layer) const {
  auto i = last_frame_paint_region_map_.find(layer->unique_id());
  if (i != last_frame_paint_region_map_.end()) {
    return i->second;
  } else {
    // This is valid when Layer::PreservePaintRegion is called for retained
    // layer with zero sized parent clip (these layers are not diffed)
    return PaintRegion();
  }
}

void DiffContext::Statistics::LogStatistics() {
#if !FLUTTER_RELEASE
  FML_TRACE_COUNTER("flutter", "DiffContext", reinterpret_cast<int64_t>(this),
                    "NewPictures", new_pictures_, "PicturesTooComplexToCompare",
                    pictures_too_complex_to_compare_, "DeepComparePictures",
                    deep_compare_pictures_, "SameInstancePictures",
                    same_instance_pictures_,
                    "DifferentInstanceButEqualPictures",
                    different_instance_but_equal_pictures_);
#endif  // !FLUTTER_RELEASE
}

}  // namespace flutter
