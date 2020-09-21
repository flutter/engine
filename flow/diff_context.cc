#include "diff_context.h"
#include "flutter/flow/layers/layer.h"

namespace flutter {

SkRect PaintRegion::GetBounds() const {
  SkRect res = SkRect::MakeEmpty();
  for (const auto& r : *this) {
    res.join(r);
  }
  return res;
}

DiffContext::DiffContext(double frame_device_pixel_ratio)
    : rects_(std::make_shared<std::vector<SkRect>>()),
      frame_device_pixel_ratio_(frame_device_pixel_ratio) {}

void DiffContext::BeginSubtree() {
  state_stack_.push_back(state_);
  state_.rect_index_ = rects_->size();
}

void DiffContext::EndSubtree() {
  assert(!state_stack_.empty());
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

Damage DiffContext::GetDamage(
    const SkIRect& accumulated_buffer_damage) const {
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
  return res;
}

bool DiffContext::PushCullRect(const SkRect& clip) {
  return state_.cull_rect.intersect(clip);
}

void DiffContext::MarkSubtreeDirty(const PaintRegion& previous_paint_region) {
  assert(!IsSubtreeDirty());
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

void DiffContext::AddPaintRegion(const PaintRegion& region) {
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
  for (const auto& r : damage) {
    damage_.join(r);
  }
}

void DiffContext::AddDamage(const SkRect& rect) {
  damage_.join(rect);
}

}  // namespace flutter
