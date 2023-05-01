// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_COMPOSITING_SCENE_H_
#define FLUTTER_LIB_UI_COMPOSITING_SCENE_H_

#include <cstdint>
#include <memory>

#include "flutter/flow/layers/layer_tree.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace flutter {

class Scene : public RefCountedDartWrappable<Scene> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(Scene);

 public:
  ~Scene() override;
  static void create(Dart_Handle scene_handle,
                     std::shared_ptr<flutter::Layer> rootLayer,
                     uint32_t rasterizerTracingThreshold,
                     bool checkerboardRasterCacheImages,
                     bool checkerboardOffscreenLayers);

  std::unique_ptr<flutter::LayerTree> takeLayerTree(uint64_t width,
                                                    uint64_t height,
                                                    float pixel_ratio);

  Dart_Handle toImageSync(uint32_t width,
                          uint32_t height,
                          Dart_Handle raw_image_handle);

  Dart_Handle toImage(uint32_t width,
                      uint32_t height,
                      Dart_Handle raw_image_handle);

  void dispose();

 private:
  Scene(std::shared_ptr<flutter::Layer> rootLayer,
        uint32_t rasterizerTracingThreshold,
        bool checkerboardRasterCacheImages,
        bool checkerboardOffscreenLayers);

  // Returns true if `dispose()` has not been called.
  bool valid();

  void RasterizeToImage(uint32_t width,
                        uint32_t height,
                        float pixel_ratio,
                        Dart_Handle raw_image_handle);

  std::unique_ptr<LayerTree> BuildLayerTree(uint32_t width,
                                            uint32_t height,
                                            float pixel_ratio);

  flutter::LayerTree::Config layer_tree_config_;

  // Fetches the pixel ratio from view 0, or if the window doesn't exist,
  // fallback to 2.0f.
  //
  // The pixel ratio is used in toImage() and toImageSync(), and its only effect
  // is to calculate the device's physical dimension, which is used by some
  // physical shapes (see PhysicalShapeLayer).
  //
  // Physical shapes have been deprecated and should be removed soon. This
  // method aims to keep the legacy behavior in single-window Flutter, which
  // feeds the toImage and toImageSync with the pixel ratio of the only window.
  //
  // TODO(dkwingsmt): If PhysicalShapeLayer has been removed as well as
  // {Preroll,Paint}Context.frame_device_pixel_ratio, remove this method and its
  // related logic.
  // https://github.com/flutter/flutter/issues/125720
  static float defaultViewPixelRatio();
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_COMPOSITING_SCENE_H_
