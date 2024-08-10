// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_VIEW_EMBEDDER_SURFACE_POOL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_VIEW_EMBEDDER_SURFACE_POOL_H_

#include "flutter/flow/surface.h"
#include "flutter/shell/platform/android/context/android_context.h"
#include "flutter/shell/platform/android/surface/android_surface.h"

namespace flutter {

//------------------------------------------------------------------------------
/// An Overlay layer represents an `android.view.View` in the C side.
///
/// The `id` is used to uniquely identify the layer and recycle it between
/// frames.
///
struct OverlayLayer {
  OverlayLayer(int id,
               std::unique_ptr<AndroidSurface> android_surface,
               std::unique_ptr<Surface> surface);

  ~OverlayLayer();

  // A unique id to identify the overlay when it gets recycled.
  const int id;

  // A GPU surface.
  const std::unique_ptr<AndroidSurface> android_surface;

  // A GPU surface. This may change when the overlay is recycled.
  std::unique_ptr<Surface> surface;
};

class SurfacePool {
 public:
  SurfacePool();

  ~SurfacePool();

  /// @brief Returns whether the cached layers are still valid.
  ///
  /// If the frame size or layer has changed, then all layers must be
  /// destroyed and recreated.
  bool CheckLayerProperties(GrDirectContext* gr_context, SkISize frame_size);

  /// @brief Gets a layer from the pool if available.
  ///
  /// The layer is marked as used until [RecycleLayers] is called.
  std::shared_ptr<OverlayLayer> GetNextLayer();

  /// @brief Create a new overlay layer.
  ///
  /// This method can only be called on the Raster thread.
  void CreateLayer(
      GrDirectContext* gr_context,
      const AndroidContext& android_context,
      std::unique_ptr<PlatformViewAndroidJNI::OverlayMetadata> overlay_metadata,
      const std::shared_ptr<AndroidSurfaceFactory>& surface_factory);

  /// @brief Removes unused layers from the pool. Returns the unused layers.
  std::vector<std::shared_ptr<OverlayLayer>> GetUnusedLayers();

  /// @brief Marks the layers in the pool as available for reuse.
  void RecycleLayers();

  /// @brief The count of layers currently in the pool.
  size_t size() const;

  /// @brief Clears the state of the surface pool.
  ///
  /// Requires that the JNI method FlutterViewDestroyOverlaySurfaces is called
  /// separately
  void DestroyLayers();

 private:
  // The index of the entry in the layers_ vector that determines the beginning
  // of the unused layers. For example, consider the following vector:
  //  _____
  //  | 0 |
  //  |---|
  //  | 1 | <-- `available_layer_index_`
  //  |---|
  //  | 2 |
  //  |---|
  //
  //  This indicates that entries starting from 1 can be reused meanwhile the
  //  entry at position 0 cannot be reused.
  size_t available_layer_index_ = 0;

  // The layers in the pool.
  std::vector<std::shared_ptr<OverlayLayer>> layers_;

  // The frame size of the layers in the pool.
  SkISize current_frame_size_;

  // The `GrContext` that is currently used by the overlay surfaces.
  // We track this to know when the GrContext for the Flutter app has changed
  // so we can update the overlay with the new context.
  //
  // This may change when the overlay is recycled.
  intptr_t gr_context_key_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_VIEW_EMBEDDER_SURFACE_POOL_H_
