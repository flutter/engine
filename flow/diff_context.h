#ifndef FLUTTER_FLOW_DIFF_CONTEXT_H_
#define FLUTTER_FLOW_DIFF_CONTEXT_H_

#include <vector>
#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkRect.h"

namespace flutter {

struct Damage {
  // This is the damage between current and previous frame;
  // When using double or triple buffering, this damage must be acumulated
  // for each "inactive" framebuffer and then be provided as additional_damage.
  SkIRect net_damage;

  // Reflects actual change to target framebuffer; This is net_damage + damage
  // previously acumulated for the framebuffer. If embedder supports partial
  // update, this is the region that needs to be updated.
  SkIRect framebuffer_damage;
};

// Every layer has a PaintRegion that covers screen area where the layer subtree
// painted anything.
//
// The area is used when adding damage of removed or dirty later.
//
// Because there is PaintRegion for each layer, it must be able to represent
// the area with minimal overhead. This is accomplished by having one
// vector of SkRect shared between all paint regions, and each paint region
// keeping begin and end index of rects relevant to particular subtree.
//
// All rects are in screen coordinates.
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

  // Compute bounds for this region
  SkRect GetBounds() const;

  bool is_valid() const { return rects_ != nullptr; }

  // Returns true if there is a layer in subtree represented by this region
  // that performs readback
  bool has_readback() const { return has_readback_; }

 private:
  std::shared_ptr<std::vector<SkRect>> rects_;
  size_t from_ = 0;
  size_t to_ = 0;
  bool has_readback_ = false;
};

// Tracks state during tree diffing process and computes resulting damage
class DiffContext {
 public:
  explicit DiffContext(double device_pixel_aspect_ratio);

  // Starts a new subtree.
  void BeginSubtree();

  // Ends current subtree; All modifications to state (transform, cullrect,
  // dirty) will be restored
  void EndSubtree();

  // Creates subtree in current scope and closes it on scope exit
  class AutoSubtreeRestore {
    FML_DISALLOW_COPY_ASSIGN_AND_MOVE(AutoSubtreeRestore);

   public:
    explicit AutoSubtreeRestore(DiffContext* context) : context_(context) {
      context->BeginSubtree();
    }
    ~AutoSubtreeRestore() { context_->EndSubtree(); }

   private:
    DiffContext* context_;
  };

  // Pushes additional transform for current subtree
  void PushTransform(const SkMatrix& transform);

  // Pushes cull rect for current subtree
  bool PushCullRect(const SkRect& clip);

  // Returns transform matrix for current subtree
  SkMatrix GetTransform() const { return state_.transform; }

  // Return cull rect for current subtree (in local coordinates)
  SkRect GetCullRect() const { return state_.cull_rect; }

  // Sets the dirty flag on current subtree;
  //
  // previous_paint_region, which should represent region of previous subtree
  // at this level will be added to damage area
  //
  // Each paint region added to dirty subtree (through AddPaintRegion) is also
  // added to damage
  void MarkSubtreeDirty(
      const PaintRegion& previous_paint_region = PaintRegion());

  bool IsSubtreeDirty() const { return state_.dirty; }

  // Add paint region for layer; rect is in "local" (layer) coordinates
  void AddPaintRegion(const SkRect& rect);

  // Add entire paint region for current subtree
  void AddPaintRegion(const PaintRegion& region);

  // The idea of readback region is that if any part of the readback region
  // needs to be repainted, then the whole readback region must be repainted;
  void AddReadbackRegion(const SkRect& rect);

  // Returns the paint region for current subtree; Each rect in paint region is
  // in screen coordinates; The result should be set to layer's paint_region
  // before closing the subtree
  PaintRegion CurrentSubtreeRegion() const;

  // Computes final damage
  //
  // additional_damage is the previously accumulated net_damage for
  // current framebuffer
  Damage GetDamage(const SkIRect& additional_damage) const;

  double frame_device_pixel_ratio() const { return frame_device_pixel_ratio_; };

 private:
  struct State {
    State();

    bool dirty;
    SkRect cull_rect;
    SkMatrix transform;
    size_t rect_index_;
  };

  std::shared_ptr<std::vector<SkRect>> rects_;
  State state_;
  double frame_device_pixel_ratio_;
  std::vector<State> state_stack_;

  SkRect damage_ = SkRect::MakeEmpty();

  void AddDamage(const SkRect& rect);
  void AddDamage(const PaintRegion& damage);

  struct Readback {
    size_t position;
    SkRect rect;
  };

  std::vector<Readback> readbacks_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_DIFF_CONTEXT_H_
