#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/picture_layer.h"
#include "flutter/flow/testing/skia_gpu_object_layer_test.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"

namespace flutter {
namespace testing {

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

class LayerTree {
 public:
  LayerTree() : root_(std::make_shared<ContainerLayer>()) {}

  ContainerLayer* root() { return root_.get(); }
  const ContainerLayer* root() const { return root_.get(); }

  PaintRegionMap& paint_region_map() { return paint_region_map_; }
  const PaintRegionMap& paint_region_map() const { return paint_region_map_; }

 private:
  std::shared_ptr<ContainerLayer> root_;
  PaintRegionMap paint_region_map_;
};

class DiffContextTest : public ThreadTest {
 public:
  DiffContextTest();

  Damage DiffLayerTree(LayerTree& layer_tree,
                       const LayerTree& old_layer_tree,
                       const SkIRect& additional_damage = SkIRect::MakeEmpty());

  // Create picture consisting of filled rect with given color; Being able
  // to specify different color is useful to test deep comparison of pictures
  sk_sp<SkPicture> CreatePicture(const SkRect& bounds, uint32_t color);

  std::shared_ptr<PictureLayer> CreatePictureLayer(
      sk_sp<SkPicture> picture,
      const SkPoint& offset = SkPoint::Make(0, 0));

  std::shared_ptr<ContainerLayer> CreateContainerLayer(
      std::initializer_list<std::shared_ptr<Layer>> layers);

  std::shared_ptr<ContainerLayer> CreateContainerLayer(
      std::shared_ptr<Layer> l) {
    return CreateContainerLayer({l});
  }

  fml::RefPtr<SkiaUnrefQueue> unref_queue() { return unref_queue_; }

 private:
  fml::RefPtr<SkiaUnrefQueue> unref_queue_;
};

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

}  // namespace testing
}  // namespace flutter
