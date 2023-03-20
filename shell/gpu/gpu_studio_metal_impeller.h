// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_GPU_GPU_STUDIO_METAL_IMPELLER_H_
#define FLUTTER_SHELL_GPU_GPU_STUDIO_METAL_IMPELLER_H_

#include <QuartzCore/CAMetalLayer.h>

#include "flutter/flow/studio.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/impeller/aiks/aiks_context.h"
#include "flutter/impeller/renderer/renderer.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"

namespace flutter {

class SK_API_AVAILABLE_CA_METAL_LAYER GPUStudioMetalImpeller : public Studio {
 public:
  GPUStudioMetalImpeller(GPUSurfaceMetalDelegate* delegate,
                         const std::shared_ptr<impeller::Context>& context);

  // |Studio|
  ~GPUStudioMetalImpeller();

  // |Studio|
  bool IsValid() override;

 private:
  const GPUSurfaceMetalDelegate* delegate_;
  std::shared_ptr<impeller::Renderer> impeller_renderer_;
  std::shared_ptr<impeller::AiksContext> aiks_context_;

  // |Studio|
  GrDirectContext* GetContext() override;

  // |Studio|
  std::unique_ptr<GLContextResult> MakeRenderContextCurrent() override;

  // |Studio|
  bool AllowsDrawingWhenGpuDisabled() const override;

  // |Studio|
  bool EnableRasterCache() const override;

  // |Studio|
  impeller::AiksContext* GetAiksContext() const override;

  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioMetalImpeller);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_STUDIO_METAL_IMPELLER_H_
