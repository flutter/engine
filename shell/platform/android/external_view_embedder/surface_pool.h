// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_VIEW_EMBEDDER_SURFACE_POOL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_VIEW_EMBEDDER_SURFACE_POOL_H_

#include "flutter/flow/surface.h"
#include "flutter/shell/platform/android/android_surface.h"
#include "flutter/shell/platform/android/context/android_context.h"

namespace flutter {

struct OverlayLayer {
  OverlayLayer(long id,
               std::unique_ptr<AndroidSurface> android_surface,
               std::unique_ptr<Surface> surface);

  ~OverlayLayer() = default;

  const long id;

  const std::unique_ptr<AndroidSurface> android_surface;

  const std::unique_ptr<Surface> surface;

  // The `GrContext` that is currently used by the overlay surfaces.
  // We track this to know when the GrContext for the Flutter app has changed
  // so we can update the overlay with the new context.
  GrContext* gr_context;
};

// This class isn't thread safe.
class SurfacePool {
 public:
  SurfacePool() = default;
  ~SurfacePool() = default;

  // Gets a layer from the pool if available, or allocates a new one.
  // Finally, it marks the layer as used. That is, it increments
  // `available_layer_index_`.
  std::shared_ptr<OverlayLayer> GetLayer(
      GrContext* gr_context,
      std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
      std::shared_ptr<AndroidContext> android_context);

  // Gets the layers in the pool that aren't currently used.
  // This method doesn't mark the layers as unused.
  std::vector<std::shared_ptr<OverlayLayer>> GetUnusedLayers();

  // Marks the layers in the pool as available for reuse.
  void RecycleLayers();

 private:
  // The index of the entry in the layers_ vector that determines the beginning
  // of the unused layers. For example, consider the following vector:
  //  _____
  //  | 0 |
  /// |---|
  /// | 1 | <-- `available_layer_index_`
  /// |---|
  /// | 2 |
  /// |---|
  ///
  /// This indicates that entries starting from 1 can be reused meanwhile the
  /// entry at position 0 cannot be reused.
  size_t available_layer_index_ = 0;
  std::vector<std::shared_ptr<OverlayLayer>> layers_;
}

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_VIEW_EMBEDDER_SURFACE_POOL_H_
