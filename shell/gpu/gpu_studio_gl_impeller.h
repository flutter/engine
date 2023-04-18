// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_STUDIO_GL_IMPELLER_H_
#define SHELL_GPU_GPU_STUDIO_GL_IMPELLER_H_

#include "flutter/common/graphics/gl_context_switch.h"
#include "flutter/flow/studio.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/impeller/aiks/aiks_context.h"
#include "flutter/impeller/renderer/context.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"

namespace flutter {

class GPUStudioGLImpeller final : public Studio {
 public:
  explicit GPUStudioGLImpeller(
      GPUSurfaceGLDelegate* delegate,
      const std::shared_ptr<impeller::Context>& context);

  // |Studio|
  ~GPUStudioGLImpeller() override;

  // |Studio|
  bool IsValid() override;

 private:
  GPUSurfaceGLDelegate* delegate_ = nullptr;
  std::shared_ptr<impeller::AiksContext> aiks_context_;
  bool is_valid_ = false;

  // |Studio|
  GrDirectContext* GetContext() override;

  // |Studio|
  std::unique_ptr<GLContextResult> MakeRenderContextCurrent() override;

  // |Studio|
  bool ClearRenderContext() override;

  // |Studio|
  bool AllowsDrawingWhenGpuDisabled() const override;

  // |Studio|
  bool EnableRasterCache() const override;

  // |Studio|
  std::shared_ptr<impeller::AiksContext> GetAiksContext() const override;

  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioGLImpeller);
};

}  // namespace flutter

#endif  // SHELL_GPU_GPU_STUDIO_GL_IMPELLER_H_
