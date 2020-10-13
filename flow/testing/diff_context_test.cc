#include "diff_context_test.h"

namespace flutter {
namespace testing {

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

DiffContextTest::DiffContextTest()
    : unref_queue_(fml::MakeRefCounted<SkiaUnrefQueue>(
          GetCurrentTaskRunner(),
          fml::TimeDelta::FromSeconds(0))) {}

Damage DiffContextTest::DiffLayerTree(LayerTree& layer_tree,
                                      const LayerTree& old_layer_tree,
                                      const SkIRect& additional_damage) {
  DiffContext dc(1, layer_tree.paint_region_map(),
                 old_layer_tree.paint_region_map());
  layer_tree.root()->Diff(&dc, old_layer_tree.root());
  return dc.GetDamage(additional_damage);
}

sk_sp<SkPicture> DiffContextTest::CreatePicture(const SkRect& bounds,
                                                uint32_t color) {
  SkPictureRecorder recorder;
  SkCanvas* recording_canvas = recorder.beginRecording(bounds);
  recording_canvas->drawRect(bounds, SkPaint(SkColor4f::FromBytes_RGBA(color)));
  return recorder.finishRecordingAsPicture();
}

std::shared_ptr<PictureLayer> DiffContextTest::CreatePictureLayer(
    sk_sp<SkPicture> picture,
    const SkPoint& offset) {
  return std::make_shared<PictureLayer>(
      offset, SkiaGPUObject(picture, unref_queue()), false, false);
}

std::shared_ptr<ContainerLayer> DiffContextTest::CreateContainerLayer(
    std::initializer_list<std::shared_ptr<Layer>> layers) {
  auto res = std::make_shared<ContainerLayer>();
  for (const auto& l : layers) {
    res->Add(l);
  }
  return res;
}

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

}  // namespace testing
}  // namespace flutter
