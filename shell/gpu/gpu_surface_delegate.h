// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_GPU_GPU_SURFACE_DELEGATE_H_
#define FLUTTER_SHELL_GPU_GPU_SURFACE_DELEGATE_H_

#include "flutter/flow/embedded_views.h"

namespace flutter {

class GPUSurfaceDelegate {
 public:
  virtual ~GPUSurfaceDelegate();

  //----------------------------------------------------------------------------
  /// @brief      Gets the pixel geometry of the surface, i.e. whether the
  ///             physical display is an LCD using BGR or RGB pixel layout and
  ///             whether it is horizontally or vertically oriented.
  ///             A change in this value will invalidate the current SkSurface.
  ///             This value must be set to kUnknown_SkPixelGeometry in the
  ///             case of raster backed surfaces such as a software surface.
  ///
  /// @return     The SkPixelGeometry of the current display the surface is
  ///             rendered on. If the surface is being rendered on multiple
  ///             displays with differing pixel geometry, it is up to the
  ///             platform to pick which one to use.
  ///
  virtual SkPixelGeometry GetPixelGeometry() const;

  //----------------------------------------------------------------------------
  /// @brief      Gets the view embedder that controls how the Flutter layer
  ///             hierarchy split into multiple chunks should be composited back
  ///             on-screen. This field is optional and the Flutter rasterizer
  ///             will render into a single on-screen surface if this call
  ///             returns a null external view embedder. This happens on the GPU
  ///             thread.
  ///
  /// @return     The external view embedder, or, null if Flutter is rendering
  ///             into a single on-screen surface.
  ///
  virtual ExternalViewEmbedder* GetExternalViewEmbedder() = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_SURFACE_DELEGATE_H_
