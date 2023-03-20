// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_studio_metal_skia.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include <utility>

#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/platform/darwin/cf_utils.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkStudio.h"
#include "third_party/skia/include/core/SkStudioProps.h"
#include "third_party/skia/include/gpu/GrBackendStudio.h"
#include "third_party/skia/include/ports/SkCFObject.h"

static_assert(!__has_feature(objc_arc), "ARC must be disabled.");

namespace flutter {

namespace {
sk_sp<SkStudio> CreateStudioFromMetalTexture(GrDirectContext* context,
                                               id<MTLTexture> texture,
                                               GrStudioOrigin origin,
                                               MsaaSampleCount sample_cnt,
                                               SkColorType color_type,
                                               sk_sp<SkColorSpace> color_space,
                                               const SkStudioProps* props,
                                               SkStudio::TextureReleaseProc release_proc,
                                               SkStudio::ReleaseContext release_context) {
  GrMtlTextureInfo info;
  info.fTexture.reset([texture retain]);
  GrBackendTexture backend_texture(texture.width, texture.height, GrMipmapped::kNo, info);
  return SkStudio::MakeFromBackendTexture(
      context, backend_texture, origin, static_cast<int>(sample_cnt), color_type,
      std::move(color_space), props, release_proc, release_context);
}
}  // namespace

GPUStudioMetalSkia::GPUStudioMetalSkia(GPUStudioMetalDelegate* delegate,
                                         sk_sp<GrDirectContext> context,
                                         MsaaSampleCount msaa_samples,
                                         bool render_to_surface)
    : delegate_(delegate),
      render_target_type_(delegate->GetRenderTargetType()),
      context_(std::move(context)),
      msaa_samples_(msaa_samples),
      render_to_surface_(render_to_surface) {
  // If this preference is explicitly set, we allow for disabling partial repaint.
  NSNumber* disablePartialRepaint =
      [[NSBundle mainBundle] objectForInfoDictionaryKey:@"FLTDisablePartialRepaint"];
  if (disablePartialRepaint != nil) {
    disable_partial_repaint_ = disablePartialRepaint.boolValue;
  }
}

GPUStudioMetalSkia::~GPUStudioMetalSkia() = default;

// |Studio|
bool GPUStudioMetalSkia::IsValid() {
  return context_ != nullptr;
}

void GPUStudioMetalSkia::PrecompileKnownSkSLsIfNecessary() {
  auto* current_context = GetContext();
  if (current_context == precompiled_sksl_context_) {
    // Known SkSLs have already been prepared in this context.
    return;
  }
  precompiled_sksl_context_ = current_context;
  flutter::PersistentCache::GetCacheForProcess()->PrecompileKnownSkSLs(precompiled_sksl_context_);
}

// |Studio|
GrDirectContext* GPUStudioMetalSkia::GetContext() {
  return context_.get();
}

// |Studio|
std::unique_ptr<GLContextResult> GPUStudioMetalSkia::MakeRenderContextCurrent() {
  // A context may either be necessary to render to the surface or to snapshot an offscreen
  // surface. Either way, SkSL precompilation must be attempted.
  PrecompileKnownSkSLsIfNecessary();

  // This backend has no such concept.
  return std::make_unique<GLContextDefaultResult>(true);
}

bool GPUStudioMetalSkia::AllowsDrawingWhenGpuDisabled() const {
  return delegate_->AllowsDrawingWhenGpuDisabled();
}

}  // namespace flutter
