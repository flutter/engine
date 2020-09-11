#ifndef FLUTTER_FLOW_CONTEXT_H_
#define FLUTTER_FLOW_CONTEXT_H_

#include <unordered_set>
#include "flutter/flow/embedded_views.h"
#include "third_party/skia/include/core/SkRect.h"

class SkImageFilter;

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

  void PushLayerContribution(const Layer* layer,
                             LayerComparator comparator,
                             const SkMatrix& matrix,
                             const PrerollContext& preroll_context,
                             size_t index = size_t(-1));

  size_t layer_entries_count() const { return layer_entries_.size(); }

  // Adjusts the paint bounds of layer entries within given range according to
  // filter bounds
  bool ApplyImageFilter(size_t from,
                        size_t count,
                        const SkImageFilter* filter,
                        const SkMatrix& matrix,
                        const SkRect& bounds);

  struct DamageResult {
    DamageArea area;
    std::unique_ptr<FrameDescription> frame_description;
  };

  DamageResult FinishFrame();

 private:
  // Represents a layer contribution to screen contents
  // LayerContribution can compare itself with a LayerContribution from past
  // frame to determine if the content they'd produce is identical
  // Diffing set of LayerContributions will give us damage area
  struct LayerContribution {
    SkRect paint_bounds;  // in screen coordinates
    std::shared_ptr<const Layer> layer;
    LayerComparator comparator;

    std::vector<std::shared_ptr<Mutator>> mutators;
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

 public:
  class FrameDescription {
    SkISize layer_tree_size;
    DamageContext::LayerContributionSet entries;
    friend class DamageContext;
  };
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_CONTEXT_H_
