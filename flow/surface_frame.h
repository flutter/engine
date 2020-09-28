// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_SURFACE_FRAME_H_
#define FLUTTER_FLOW_SURFACE_FRAME_H_

#include <memory>

#include "flutter/flow/gl_context_switch.h"
#include "flutter/fml/macros.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {

// This class represents a frame that has been fully configured for the
// underlying client rendering API. A frame may only be submitted once.
class SurfaceFrame {
 public:
  using SubmitCallback =
      std::function<bool(const SurfaceFrame& surface_frame, SkCanvas* canvas)>;

  // Information about the underlying framebuffer
  struct FramebufferInfo {
    // Indicates whether or not the surface supports pixel readback as used in
    // circumstances such as a BackdropFilter.
    bool supports_readback = false;

    // This is area of framebuffer that lags behind the front buffer. When doing
    // partial redraw, this area must be repainted alongside with any additional
    // area; If not set, entire frame must be repainted
    std::vector<SkIRect> existing_damage;

    // Specifies the minimum alignment for outermost clip rect. Some devices
    // with tile based rendering might incur additional overhead when rendering
    // in the middle of existing tiles
    int partial_repaint_clip_alignment = 1;
  };

  SurfaceFrame(sk_sp<SkSurface> surface,
               FramebufferInfo framebuffer_info,
               const SubmitCallback& submit_callback);

  SurfaceFrame(sk_sp<SkSurface> surface,
               FramebufferInfo framebuffer_info,
               const SubmitCallback& submit_callback,
               std::unique_ptr<GLContextResult> context_result);

  ~SurfaceFrame();

  struct SubmitInfo {
    // The surface damage for frame n is the difference between frame n and
    // frame (n-1), and represents the area that a compositor must recompose.
    //
    // Corresponds to EGL_KHR_swap_buffers_with_damage
    std::vector<SkIRect> surface_damage;

    // The buffer damage for a frame is the area changed since that same buffer
    // was last used. If the buffer has not been used before, the buffer damage
    // is the entire area of the buffer.
    //
    // Corresponds to EGL_KHR_partial_update
    std::vector<SkIRect> buffer_damage;
  };

  bool Submit(SubmitInfo submit_info = SubmitInfo());

  bool IsSubmitted() const;

  SkCanvas* SkiaCanvas();

  sk_sp<SkSurface> SkiaSurface() const;

  const FramebufferInfo& framebuffer_info() const { return framebuffer_info_; }
  const SubmitInfo& submit_info() const { return submit_info_; }

 private:
  bool submitted_ = false;
  sk_sp<SkSurface> surface_;
  FramebufferInfo framebuffer_info_;
  SubmitInfo submit_info_;
  SubmitCallback submit_callback_;
  std::unique_ptr<GLContextResult> context_result_;

  bool PerformSubmit();

  FML_DISALLOW_COPY_AND_ASSIGN(SurfaceFrame);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_SURFACE_FRAME_H_
