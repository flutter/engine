// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_GPU_GPU_SURFACE_METAL_H_
#define FLUTTER_SHELL_GPU_GPU_SURFACE_METAL_H_

#include "flutter/common/graphics/msaa_sample_count.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

class SK_API_AVAILABLE_CA_METAL_LAYER GPUStudioMetalSkia : public Studio {
 public:
  GPUStudioMetalSkia(GPUStudioMetalDelegate* delegate,
                      sk_sp<GrDirectContext> context,
                      MsaaSampleCount msaa_samples,
                      bool render_to_surface = true);

  // |Studio|
  ~GPUStudioMetalSkia();

  // |Studio|
  bool IsValid() override;

 private:
  const GPUStudioMetalDelegate* delegate_;
  const MTLRenderTargetType render_target_type_;
  sk_sp<GrDirectContext> context_;
  GrDirectContext* precompiled_sksl_context_ = nullptr;
  MsaaSampleCount msaa_samples_ = MsaaSampleCount::kNone;
  // TODO(38466): Refactor GPU surface APIs take into account the fact that an
  // external view embedder may want to render to the root surface. This is a
  // hack to make avoid allocating resources for the root surface when an
  // external view embedder is present.
  bool render_to_surface_ = true;
  bool disable_partial_repaint_ = false;

  // Accumulated damage for each framebuffer; Key is address of underlying
  // MTLTexture for each drawable
  std::map<uintptr_t, SkIRect> damage_;

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

#endif  // FLUTTER_SHELL_GPU_GPU_SURFACE_METAL_H_
