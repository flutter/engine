// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/external_view_embedder/surface_pool.h"

namespace flutter {

OverlayLayer::OverlayLayer(long id,
                           std::unique_ptr<AndroidSurface> android_surface,
                           std::unique_ptr<Surface> surface)
    : id(id),
      android_surface(std::move(android_surface)),
      surface(std::move(surface)){};

std::shared_ptr<OverlayLayer> SurfacePool::GetLayer(
    GrContext* gr_context,
    std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
    std::shared_ptr<AndroidContext> android_context) {
  // Allocate a new surface if there isn't one available.
  if (available_layer_index_ >= layers_.size()) {
    std::unique_ptr<AndroidSurface> android_surface =
        AndroidSurface::Create(android_context, jni_facade);
    // TODO(egarciad): jni_facade->FlutterViewCreateOverlaySurface(..)
    // Then, android_surface->SetNativeWindow(overlay_layer->GetWindow())
    // https://github.com/flutter/flutter/issues/55270
    std::unique_ptr<Surface> surface =
        android_surface->CreateGPUSurface(gr_context);
    std::shared_ptr<OverlayLayer> layer =
        std::make_shared<OverlayLayer>(overlay_layer->GetId(),      //
                                       std::move(android_surface),  //
                                       std::move(surface)           //
        );
    layer->gr_context = gr_context;
    layers_.push_back(layer);
  }
  std::shared_ptr<OverlayLayer> layer = layers_[available_layer_index_];
  // Since the surfaces are recycled, it's possible that the GrContext is
  // different.
  if (gr_context != layer->gr_context) {
    layer->gr_context = gr_context;
    // The overlay already exists, but the GrContext was changed so we need to
    // recreate the rendering surface with the new GrContext.
    std::unique_ptr<Surface> surface =
        layer->android_surface->CreateGPUSurface(gr_context);
    layer->surface = std::move(surface);
  }
  available_layer_index_++;
  return layer;
}

void SurfacePool::RecycleLayers() {
  available_layer_index_ = 0;
}

std::vector<std::shared_ptr<OverlayLayer>> SurfacePool::GetUnusedLayers() {
  std::vector<std::shared_ptr<OverlayLayer>> results;
  for (size_t i = available_layer_index_; i < layers_.size(); i++) {
    results.push_back(layers_[i]);
  }
  return results;
}

}  // namespace flutter
