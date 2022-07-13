// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/compositing/scene.h"

#include "flutter/fml/trace_event.h"
#include "flutter/lib/ui/painting/display_list_deferred_image_gpu.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/lib/ui/painting/picture.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_configuration.h"
#include "flutter/lib/ui/window/window.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, Scene);

#define FOR_EACH_BINDING(V) \
  V(Scene, toImageSync)     \
  V(Scene, toImage)         \
  V(Scene, dispose)

DART_BIND_ALL(Scene, FOR_EACH_BINDING)

void Scene::create(Dart_Handle scene_handle,
                   std::shared_ptr<flutter::Layer> rootLayer,
                   uint32_t rasterizerTracingThreshold,
                   bool checkerboardRasterCacheImages,
                   bool checkerboardOffscreenLayers) {
  auto scene = fml::MakeRefCounted<Scene>(
      std::move(rootLayer), rasterizerTracingThreshold,
      checkerboardRasterCacheImages, checkerboardOffscreenLayers);
  scene->AssociateWithDartWrapper(scene_handle);
}

Scene::Scene(std::shared_ptr<flutter::Layer> rootLayer,
             uint32_t rasterizerTracingThreshold,
             bool checkerboardRasterCacheImages,
             bool checkerboardOffscreenLayers) {
  // Currently only supports a single window.
  auto viewport_metrics = UIDartState::Current()
                              ->platform_configuration()
                              ->get_window(0)
                              ->viewport_metrics();

  layer_tree_ = std::make_unique<LayerTree>(
      SkISize::Make(viewport_metrics.physical_width,
                    viewport_metrics.physical_height),
      static_cast<float>(viewport_metrics.device_pixel_ratio));
  layer_tree_->set_root_layer(std::move(rootLayer));
  layer_tree_->set_rasterizer_tracing_threshold(rasterizerTracingThreshold);
  layer_tree_->set_checkerboard_raster_cache_images(
      checkerboardRasterCacheImages);
  layer_tree_->set_checkerboard_offscreen_layers(checkerboardOffscreenLayers);
}

Scene::~Scene() {}

void Scene::dispose() {
  layer_tree_.reset();
  ClearDartWrapper();
}

Dart_Handle Scene::toImageSync(uint32_t width,
                               uint32_t height,
                               Dart_Handle raw_image_handle) {
  TRACE_EVENT0("flutter", "Scene::toImageSync");
  auto* dart_state = UIDartState::Current();
  auto snapshot_delegate = dart_state->GetSnapshotDelegate();

  if (!layer_tree_) {
    return tonic::ToDart("Scene did not contain a layer tree.");
  }

  Scene::RasterizeToImageSync(width, height, raw_image_handle);

  return Dart_Null();
}

Dart_Handle Scene::toImage(uint32_t width,
                           uint32_t height,
                           Dart_Handle raw_image_callback) {
  TRACE_EVENT0("flutter", "Scene::toImage");

  if (!layer_tree_) {
    return tonic::ToDart("Scene did not contain a layer tree.");
  }
  auto picture = layer_tree_->Flatten(SkRect::MakeWH(width, height));
  if (!picture) {
    return tonic::ToDart("Could not flatten scene into a layer tree.");
  }

  return Picture::RasterizeToImage(picture, width, height, raw_image_callback);
}

void Scene::RasterizeToImageSync(uint32_t width,
                                 uint32_t height,
                                 Dart_Handle raw_image_handle) {
  auto* dart_state = UIDartState::Current();
  auto unref_queue = dart_state->GetSkiaUnrefQueue();
  auto snapshot_delegate = dart_state->GetSnapshotDelegate();
  auto raster_task_runner = dart_state->GetTaskRunners().GetRasterTaskRunner();

  auto image = CanvasImage::Create();
  auto dl_image = DlDeferredImageGPU::Make(SkISize::Make(width, height));
  image->set_image(dl_image);

  auto layer_tree = takeLayerTree();

  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner,
      [snapshot_delegate, unref_queue, dl_image = std::move(dl_image),
       layer_tree = std::move(layer_tree.get()), width, height]() {
        auto display_list = layer_tree->FlattenWithContext(
            SkRect::MakeWH(width, height),
            snapshot_delegate.get()->GetTextureRegistry());

        sk_sp<SkImage> sk_image;
        std::string error;
        std::tie(sk_image, error) = snapshot_delegate->MakeGpuImage(
            display_list, dl_image->dimensions());
        if (sk_image) {
          dl_image->set_image(std::move(sk_image));
        } else {
          dl_image->set_error(std::move(error));
        }
      });

  image->AssociateWithDartWrapper(raw_image_handle);
}

std::unique_ptr<flutter::LayerTree> Scene::takeLayerTree() {
  return std::move(layer_tree_);
}

}  // namespace flutter
