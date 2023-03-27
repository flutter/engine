// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_STUDIO_GL_SKIA_H_
#define SHELL_GPU_GPU_STUDIO_GL_SKIA_H_

#include <functional>
#include <memory>

#include "flutter/common/graphics/gl_context_switch.h"
#include "flutter/flow/embedded_views.h"
#include "flutter/flow/studio.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"

#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

class GPUStudioGLSkia : public Studio {
 public:
  static sk_sp<GrDirectContext> MakeGLContext(GPUSurfaceGLDelegate* delegate);

  GPUStudioGLSkia(const sk_sp<GrDirectContext>& gr_context,
                  GPUSurfaceGLDelegate* delegate);

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
  GPUSurfaceGLDelegate* delegate_;
  sk_sp<GrDirectContext> context_;
  bool valid_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioGLSkia);
};

}  // namespace flutter

#endif  // SHELL_GPU_GPU_STUDIO_GL_SKIA_H_
