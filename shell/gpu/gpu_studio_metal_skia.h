// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_GPU_GPU_STUDIO_METAL_H_
#define FLUTTER_SHELL_GPU_GPU_STUDIO_METAL_H_

#include "flutter/common/graphics/msaa_sample_count.h"
#include "flutter/flow/studio.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

class SK_API_AVAILABLE_CA_METAL_LAYER GPUStudioMetalSkia : public Studio {
 public:
  GPUStudioMetalSkia(GPUSurfaceMetalDelegate* delegate,
                     sk_sp<GrDirectContext> context,
                     std::shared_ptr<GPUSurfaceMetalDelegate::SkSLPrecompiler>
                         sksl_precompiler);

  // |Studio|
  ~GPUStudioMetalSkia();

  // |Studio|
  bool IsValid() override;

 private:
  const GPUSurfaceMetalDelegate* delegate_;
  sk_sp<GrDirectContext> context_;
  std::shared_ptr<GPUSurfaceMetalDelegate::SkSLPrecompiler> sksl_precompiler_;

  // |Studio|
  GrDirectContext* GetContext() override;

  // |Studio|
  std::unique_ptr<GLContextResult> MakeRenderContextCurrent() override;

  // |Studio|
  bool AllowsDrawingWhenGpuDisabled() const override;

  void PrecompileKnownSkSLsIfNecessary();

  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioMetalSkia);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_STUDIO_METAL_H_
