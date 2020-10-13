#ifndef FLUTTER_FLOW_DIFF_CONTEXT_H_
#define FLUTTER_FLOW_DIFF_CONTEXT_H_

#include <map>
#include <vector>
#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkRect.h"

namespace flutter {

#ifndef NDEBUG
#define FLUTTER_ENABLE_DIFF_CONTEXT
#endif

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

class Layer;

struct Damage {
  // This is the damage between current and previous frame;
  // If embedder supports partial update, this is the region that needs to be
  // swapped.
  SkIRect surface_damage;

  // Reflects actual change to target framebuffer; This is surface_damage +
  // damage previously acumulated for target framebuffer.
  // All drawing will be clipped to this region. Knowing the affected area
  // upfront may be useful for tile based GPUs.
  SkIRect buffer_damage;
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
    FML_DCHECK(is_valid());
    return rects_->begin() + from_;
  }

  std::vector<SkRect>::const_iterator end() const {
    FML_DCHECK(is_valid());
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

using PaintRegionMap = std::map<uint64_t, PaintRegion>;

// Tracks state during tree diffing process and computes resulting damage
class DiffContext {
 public:
  explicit DiffContext(double device_pixel_aspect_ratio,
                       PaintRegionMap& this_frame_paint_region_map,
                       const PaintRegionMap& last_frame_paint_region_map);

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
  // additional_damage is the previously accumulated surface_damage for
  // current framebuffer
  Damage GetDamage(const SkIRect& additional_damage) const;

  double frame_device_pixel_ratio() const { return frame_device_pixel_ratio_; };

  // Adds the region to current damage
  void AddDamage(const PaintRegion& damage);

  void SetLayerPaintRegion(const Layer* layer, const PaintRegion& region);

  PaintRegion GetOldLayerPaintRegion(const Layer* layer) const;

  class Statistics {
   public:
    // Picture replaced by different picture
    void AddNewPicture() { ++new_pictures_; }

    // Picture that would require deep comparison but was considered too complex
    // to serialize and thus was treated as new picture
    void AddPictureTooComplexToCompare() { ++picture_too_complex_to_compare_; }

    // Picture that has identical instance between frames
    void AddSameInstancePicture() { ++same_instance_pictures_; };

    // Pictures that had to be serialized to compare for equality
    void AddDeepComparePicture() { ++deep_compare_pictures_; }

    // Pictures that had to be serialized to compare (different instances),
    // but were equal
    void AddDifferentInstanceButEqualPicture() {
      ++difference_instance_but_equal_pictures_;
    };

    // Logs the statistics to trace counter
    void LogStatistics();

   private:
    int new_pictures_ = 0;
    int picture_too_complex_to_compare_ = 0;
    int same_instance_pictures_ = 0;
    int deep_compare_pictures_ = 0;
    int difference_instance_but_equal_pictures_ = 0;
  };

  Statistics& statistics() { return statistics_; }

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

  PaintRegionMap& this_frame_paint_region_map_;
  const PaintRegionMap& last_frame_paint_region_map_;

  void AddDamage(const SkRect& rect);

  struct Readback {
    size_t position;
    SkRect rect;
  };

  std::vector<Readback> readbacks_;
  Statistics statistics_;
};

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

}  // namespace flutter

#endif  // FLUTTER_FLOW_DIFF_CONTEXT_H_
