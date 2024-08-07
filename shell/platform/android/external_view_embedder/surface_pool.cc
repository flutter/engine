// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/external_view_embedder/surface_pool.h"

#include <utility>

namespace flutter {

OverlayLayer::OverlayLayer(int id,
                           std::unique_ptr<AndroidSurface> android_surface,
                           std::unique_ptr<Surface> surface)
    : id(id),
      android_surface(std::move(android_surface)),
      surface(std::move(surface)){};

OverlayLayer::~OverlayLayer() = default;

SurfacePool::SurfacePool() = default;

SurfacePool::~SurfacePool() = default;

bool SurfacePool::CheckLayerProperties(GrDirectContext* gr_context,
                                       SkISize frame_size) {
  intptr_t gr_context_key = reinterpret_cast<intptr_t>(gr_context);
  bool destroy_all_layers =
      gr_context_key != gr_context_key_ || frame_size != current_frame_size_;
  current_frame_size_ = frame_size;
  gr_context_key_ = gr_context_key;
  return destroy_all_layers;
}

std::shared_ptr<OverlayLayer> SurfacePool::GetNextLayer() {
  if (available_layer_index_ >= layers_.size()) {
    return nullptr;
  }
  return layers_[available_layer_index_++];
}

void SurfacePool::CreateLayer(
    GrDirectContext* gr_context,
    const AndroidContext& android_context,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade,
    const std::shared_ptr<AndroidSurfaceFactory>& surface_factory) {
  std::unique_ptr<AndroidSurface> android_surface =
      surface_factory->CreateSurface();

  FML_CHECK(android_surface && android_surface->IsValid())
      << "Could not create an OpenGL, Vulkan or Software surface to set up "
         "rendering.";

  std::unique_ptr<PlatformViewAndroidJNI::OverlayMetadata> java_metadata =
      jni_facade->FlutterViewCreateOverlaySurface();

  FML_CHECK(java_metadata->window);
  android_surface->SetNativeWindow(java_metadata->window);

  std::unique_ptr<Surface> surface =
      android_surface->CreateGPUSurface(gr_context);

  std::shared_ptr<OverlayLayer> layer =
      std::make_shared<OverlayLayer>(java_metadata->id,           //
                                     std::move(android_surface),  //
                                     std::move(surface)           //
      );
  layers_.push_back(layer);
}

void SurfacePool::RecycleLayers() {
  available_layer_index_ = 0;
}

void SurfacePool::DestroyLayers(
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade) {
  if (layers_.empty()) {
    return;
  }
  jni_facade->FlutterViewDestroyOverlaySurfaces();
  layers_.clear();
  available_layer_index_ = 0;
}

size_t SurfacePool::size() const {
  return layers_.size();
}

std::vector<std::shared_ptr<OverlayLayer>> SurfacePool::GetUnusedLayers() {
  std::vector<std::shared_ptr<OverlayLayer>> results;
  for (size_t i = available_layer_index_; i < layers_.size(); i++) {
    results.push_back(layers_[i]);
  }
  return results;
}

}  // namespace flutter
