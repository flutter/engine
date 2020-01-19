#ifndef FLUTTER_SHELL_GPU_GPU_SURFACE_DELEGATE_H_
#define FLUTTER_SHELL_GPU_GPU_SURFACE_DELEGATE_H_

#include "flutter/flow/embedded_views.h"

namespace flutter {

class GPUSurfaceDelegate {
 public:
  virtual ~GPUSurfaceDelegate() {}

  // Gets the view embedder that controls how the Flutter layer hierarchy split
  // into multiple chunks should be composited back on-screen. This field is
  // optional and the Flutter rasterizer will render into a single on-screen
  // surface if this call returns a null external view embedder.
  //
  // This happens on the gpu thread.
  virtual ExternalViewEmbedder* GetExternalViewEmbedder() = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_SURFACE_DELEGATE_H_
