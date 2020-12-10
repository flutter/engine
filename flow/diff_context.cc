#include "flutter/flow/diff_context.h"
#include "flutter/flow/layers/layer.h"

namespace flutter {

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

DiffContext::DiffContext(SkISize frame_size,
                         double frame_device_pixel_ratio,
                         PaintRegionMap& this_frame_paint_region_map,
                         const PaintRegionMap& last_frame_paint_region_map)
    : rects_(std::make_shared<std::vector<SkRect>>()),
      frame_size_(frame_size),
      frame_device_pixel_ratio_(frame_device_pixel_ratio),
      this_frame_paint_region_map_(this_frame_paint_region_map),
      last_frame_paint_region_map_(last_frame_paint_region_map) {}

void DiffContext::BeginSubtree() {
  state_stack_.push_back(state_);
  state_.rect_index_ = rects_->size();
}

void DiffContext::EndSubtree() {
  FML_DCHECK(!state_stack_.empty());
  state_ = std::move(state_stack_.back());
  state_stack_.pop_back();
}

DiffContext::State::State()
    : dirty(false), cull_rect(kGiantRect), rect_index_(0) {}

void DiffContext::PushTransform(const SkMatrix& transform) {
  state_.transform.preConcat(transform);
  SkMatrix inverse_transform;
  // Perspective projections don't produce rectangles that are useful for
  // culling for some reason.
  if (!transform.hasPerspective() && transform.invert(&inverse_transform)) {
    inverse_transform.mapRect(&state_.cull_rect);
  } else {
    state_.cull_rect = kGiantRect;
  }
}

Damage DiffContext::GetDamage(const SkIRect& accumulated_buffer_damage) const {
  SkRect framebuffer = SkRect::Make(accumulated_buffer_damage);
  framebuffer.join(damage_);
  SkRect net(damage_);

  for (const auto& r : readbacks_) {
    if (r.rect.intersects(net)) {
      net.join(r.rect);
    }
    if (r.rect.intersects(framebuffer)) {
      framebuffer.join(r.rect);
    }
  }

  Damage res;
  framebuffer.roundOut(&res.buffer_damage);
  net.roundOut(&res.surface_damage);

  SkIRect frame_clip = SkIRect::MakeSize(frame_size_);
  res.buffer_damage.intersect(frame_clip);
  res.surface_damage.intersect(frame_clip);
  return res;
}

bool DiffContext::PushCullRect(const SkRect& clip) {
  return state_.cull_rect.intersect(clip);
}

void DiffContext::MarkSubtreeDirty(const PaintRegion& previous_paint_region) {
  FML_DCHECK(!IsSubtreeDirty());
  if (previous_paint_region.is_valid()) {
    AddDamage(previous_paint_region);
  }
  state_.dirty = true;
}

void DiffContext::AddPaintRegion(const SkRect& rect) {
  SkRect r(rect);
  if (r.intersect(state_.cull_rect)) {
    state_.transform.mapRect(&r);
    if (!r.isEmpty()) {
      rects_->push_back(r);
      if (IsSubtreeDirty()) {
        AddDamage(r);
      }
    }
  }
}

void DiffContext::AddExistingPaintRegion(const PaintRegion& region) {
  // Adding paint region for retained layer implies that current subtree is not
  // dirty
  FML_DCHECK(!IsSubtreeDirty());
  FML_DCHECK(region.is_valid());
  rects_->insert(rects_->end(), region.begin(), region.end());
}

void DiffContext::AddReadbackRegion(const SkRect& rect) {
  SkRect r(rect);
  state_.transform.mapRect(&r);
  Readback readback;
  readback.rect = r;
  readback.position = rects_->size();
  // Push empty rect as a placeholder for position in current subtree
  rects_->push_back(SkRect::MakeEmpty());
  readbacks_.push_back(std::move(readback));
}

PaintRegion DiffContext::CurrentSubtreeRegion() const {
  bool has_readback = std::any_of(
      readbacks_.begin(), readbacks_.end(),
      [&](const Readback& r) { return r.position >= state_.rect_index_; });
  return PaintRegion(rects_, state_.rect_index_, rects_->size(), has_readback);
}

void DiffContext::AddDamage(const PaintRegion& damage) {
  FML_DCHECK(damage.is_valid());
  for (const auto& r : damage) {
    damage_.join(r);
  }
}

void DiffContext::AddDamage(const SkRect& rect) {
  damage_.join(rect);
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
    FML_CHECK(false) << "Old layer doesn't have paint region";
    return PaintRegion();
  }
}

void DiffContext::Statistics::LogStatistics() {
#if !FLUTTER_RELEASE
  FML_TRACE_COUNTER("flutter", "DiffContext", reinterpret_cast<int64_t>(this),
                    "NewPictures", new_pictures_, "PicturesTooComplexToCompare",
                    picture_too_complex_to_compare_, "DeepComparePictures",
                    deep_compare_pictures_, "SameInstancePictures",
                    same_instance_pictures_,
                    "DifferentInstanceButEqualPictures",
                    difference_instance_but_equal_pictures_);
#endif  // !FLUTTER_RELEASE
}

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

}  // namespace flutter
