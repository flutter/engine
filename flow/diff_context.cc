#include "diff_context.h"

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

DiffContext::SubtreeHandle DiffContext::BeginSubtree() {
  State copy(state_);
  state_.rect_index_ = rects_->size();
  return SubtreeHandle(this, copy);
}

void DiffContext::EndSubtree(State&& state) {
  // we're closing dirty subtree, consider current subtree region damage
  if (state_.dirty && !state.dirty) {
    AddDamage(CurrentSubtreeRegion());
  }
  state_ = std::move(state);
}

DiffContext::SubtreeHandle::SubtreeHandle(DiffContext* context,
                                          const State& previous_state)
    : context_(context), previous_state_(previous_state) {}

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

SkRect DiffContext::GetDamage() {
  for (const auto& r : readbacks_) {
    if (r.rect.intersects(damage_)) {
      damage_.join(r.rect);
      (*rects_)[r.position] = r.rect;
    }
  }
  return damage_;
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
    }
  }
}

void DiffContext::AddPaintRegion(const PaintRegion& region) {
  for (const auto& r : region) {
    rects_->push_back(r);
  }
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

}  // namespace flutter
