#ifndef FLUTTER_FLOW_DIFF_CONTEXT_H_
#define FLUTTER_FLOW_DIFF_CONTEXT_H_

#include <vector>
#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkRect.h"

namespace flutter {

// Represents a region on screen to which layer subtree has painted anything
class PaintRegion {
 public:
  PaintRegion() {}
  PaintRegion(std::shared_ptr<std::vector<SkRect>> rects,
              size_t from,
              size_t to,
              bool has_readback)
      : rects_(rects), from_(from), to_(to), has_readback_(has_readback) {}

  std::vector<SkRect>::const_iterator begin() const {
    assert(is_valid());
    return rects_->begin() + from_;
  }

  std::vector<SkRect>::const_iterator end() const {
    assert(is_valid());
    return rects_->begin() + to_;
  }

  SkRect GetBounds() const;

  bool is_valid() const { return rects_ != nullptr; }

  bool has_readback() const { return has_readback_; }

 private:
  std::shared_ptr<std::vector<SkRect>> rects_;
  size_t from_ = 0;
  size_t to_ = 0;
  bool has_readback_ = false;
};

class DiffContext {
 public:
  explicit DiffContext(double device_pixel_aspect_ratio);

  class SubtreeHandle;

  // Starts a new subtree. Subtree is ended when handle gets out of scope;
  // All modifications (transform, cull rect) will be restored
  [[nodiscard]] SubtreeHandle BeginSubtree();

  // Pushes additional transform for current subtree
  void PushTransform(const SkMatrix& transform);

  // Pushes cull rect for current subtree
  bool PushCullRect(const SkRect& clip);

  SkMatrix GetTransform() const { return state_.transform; }

  SkRect GetCullRect() const { return state_.cull_rect; }

  // Sets the dirty flag on current subtree;
  // When the subtree is closed, entire paint region of the subtree will be
  // added to damage; Additionally previous_paint_region, which is supposed
  // to be paint region of previous subtree, will also be added to damage.
  void MarkSubtreeDirty(
      const PaintRegion& previous_paint_region = PaintRegion());

  bool IsSubtreeDirty() const { return state_.dirty; }

  // Add paint region for layer; rect must be in "local" (layer) coordinates
  void AddPaintRegion(const SkRect& rect);

  // Add entire paint region for current subtree
  void AddPaintRegion(const PaintRegion& region);

  // The idea of readback region is that if any part of the readback region
  // needs to be repainted, then the whole readback region must be repainted;
  void AddReadbackRegion(const SkRect& rect);

  PaintRegion CurrentSubtreeRegion() const;

  // Computes final damage
  SkRect GetDamage();

  double frame_device_pixel_ratio() const { return frame_device_pixel_ratio_; };

 private:
  static constexpr SkRect kGiantRect =
      SkRect::MakeLTRB(-1E9F, -1E9F, 1E9F, 1E9F);
  struct State {
    bool dirty = false;
    SkRect cull_rect = kGiantRect;
    SkMatrix transform;
    size_t rect_index_ = 0;
  };

  std::shared_ptr<std::vector<SkRect>> rects_;
  State state_;
  double frame_device_pixel_ratio_;

  SkRect damage_ = SkRect::MakeEmpty();

  void AddDamage(const SkRect& rect);
  void AddDamage(const PaintRegion& damage);
  void EndSubtree(State&& state);

  struct Readback {
    size_t position;
    SkRect rect;
  };

  std::vector<Readback> readbacks_;

 public:
  class SubtreeHandle {
    FML_DISALLOW_COPY_ASSIGN_AND_MOVE(SubtreeHandle);

   public:
    ~SubtreeHandle() { context_->EndSubtree(std::move(previous_state_)); }

   private:
    friend class DiffContext;
    SubtreeHandle(DiffContext* context, const State& previous_state_);
    DiffContext* context_;
    State previous_state_;
  };
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_DIFF_CONTEXT_H_
