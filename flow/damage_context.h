#ifndef FLUTTER_FLOW_CONTEXT_H_
#define FLUTTER_FLOW_CONTEXT_H_

#include <unordered_set>
#include "flutter/flow/embedded_views.h"
#include "third_party/skia/include/core/SkRect.h"

namespace flutter {

struct PrerollContext;
class Layer;

class DamageArea {
 public:
  FML_DISALLOW_COPY_AND_ASSIGN(DamageArea);

  DamageArea() = default;
  DamageArea(DamageArea&&) = default;
  DamageArea& operator=(DamageArea&&) = default;

  const SkIRect& bounds() const { return bounds_; }

  std::vector<SkIRect> GetRects() const;

  void AddRect(const SkRect& rect);
  void AddRect(const SkIRect& rect);

 private:
  SkIRect bounds_ = SkIRect::MakeEmpty();
};

class DamageContext {
 public:
  typedef bool (*LayerComparator)(const Layer* l1, const Layer* l2);

  // Opaque representation of a frame contents
  class FrameDescription;

  void InitFrame(const SkISize& frame_size,
                 const FrameDescription* previous_frame_description);

  class LayerContributionHandle;

  LayerContributionHandle AddLayerContribution(
      const Layer* layer,
      LayerComparator comparator,
      const SkMatrix& matrix,
      const SkRect& paint_bounds,
      const PrerollContext& preroll_context);

  LayerContributionHandle AddLayerContribution(
      const Layer* layer,
      LayerComparator comparator,
      const SkMatrix& matrix,
      const PrerollContext& preroll_context);

  // If any part of readback area is dirty, the whole rectangle is considered
  // dirty; At this point we can't partially repaint backdrop filter,
  // because the edges would be sampled outside of clip rect, which would
  // produce different result compared to full repaint
  void AddReadbackArea(const SkMatrix& matrix, const SkRect& area);

  bool IsDeterminingDamage() const { return previous_frame_ != nullptr; }

  struct DamageResult {
    DamageArea area;
    std::unique_ptr<FrameDescription> frame_description;
  };

  DamageResult FinishFrame();

  class LayerContributionHandle {
   public:
    void UpdatePaintBounds();

   private:
    friend class DamageContext;
    DamageContext* context_ = nullptr;
    size_t index_ = static_cast<size_t>(-1);
    SkMatrix matrix_;
  };

 private:
  // Represents a layer contribution to screen contents
  // LayerContribution can compare itself with a LayerContribution from past
  // frame to determine if the content they'd produce is identical
  // Diffing set of LayerContributions will give us damage area
  struct LayerContribution {
    SkRect paint_bounds;  // in screen coordinates
    std::shared_ptr<const Layer> layer;
    LayerComparator comparator;
    std::shared_ptr<MutatorNode> mutator_node;
    int paint_order;

    bool operator==(const LayerContribution& e) const;
    bool operator!=(const LayerContribution& e) const { return !(*this == e); }

    struct Hash {
      std::size_t operator()(const LayerContribution& e) const noexcept;
    };
  };

  using LayerContributionSet =
      std::unordered_set<LayerContribution, LayerContribution::Hash>;
  using LayerContributionList = std::vector<LayerContribution>;

  const FrameDescription* previous_frame_ = nullptr;
  SkISize current_layer_tree_size_ = SkISize::MakeEmpty();
  LayerContributionList layer_entries_;
  std::vector<SkIRect> readback_areas_;

 public:
  class FrameDescription {
    SkISize layer_tree_size;
    DamageContext::LayerContributionSet entries;
    friend class DamageContext;
  };
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_CONTEXT_H_
