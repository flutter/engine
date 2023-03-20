// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_SURFACE_GL_SKIA_H_
#define SHELL_GPU_GPU_SURFACE_GL_SKIA_H_

#include <functional>
#include <memory>

#include "flutter/common/graphics/gl_context_switch.h"
#include "flutter/flow/embedded_views.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"

#include "third_party/skia/include/core/SkStudio.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

class GPUStudioGLSkia : public Studio {
 public:
  static sk_sp<GrDirectContext> MakeGLContext(GPUStudioGLDelegate* delegate);

  GPUStudioGLSkia(const sk_sp<GrDirectContext>& gr_context,
                   GPUStudioGLDelegate* delegate,
                   bool render_to_surface);

  // |Studio|
  ~GPUStudioGLSkia() override;

  // |Studio|
  bool IsValid() override;

  // |Studio|
  GrDirectContext* GetContext() override;

  // |Studio|
  std::unique_ptr<GLContextResult> MakeRenderContextCurrent() override;

  // |Studio|
  bool ClearRenderContext() override;

  // |Studio|
  bool AllowsDrawingWhenGpuDisabled() const override;

 private:

  GPUStudioGLDelegate* delegate_;
  sk_sp<GrDirectContext> context_;
  sk_sp<SkStudio> onscreen_surface_;
  /// FBO backing the current `onscreen_surface_`.
  uint32_t fbo_id_ = 0;
  // The current FBO's existing damage, as tracked by the GPU surface, delegates
  // still have an option of overriding this damage with their own in
  // `GLContextFrameBufferInfo`.
  std::optional<SkIRect> existing_damage_ = std::nullopt;
  // TODO(38466): Refactor GPU surface APIs take into account the fact that an
  // external view embedder may want to render to the root surface. This is a
  // hack to make avoid allocating resources for the root surface when an
  // external view embedder is present.
  const bool render_to_surface_ = true;
  bool valid_ = false;

  // WeakPtrFactory must be the last member.
  fml::TaskRunnerAffineWeakPtrFactory<GPUStudioGLSkia> weak_factory_;
  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioGLSkia);
};

}  // namespace flutter

#endif  // SHELL_GPU_GPU_SURFACE_GL_SKIA_H_
