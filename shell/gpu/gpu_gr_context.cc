// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_gr_context.h"

#include "lib/fxl/logging.h"

#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace shell {
// Default maximum number of budgeted resources in the cache.
static const int kGrCacheMaxCount = 8192;

// Default maximum number of bytes of GPU memory of budgeted resources in the
// cache.
static const size_t kGrCacheMaxByteSize = 512 * (1 << 20);

GpuGrContext::GpuGrContext(bool createContext) {
  GrContextOptions options;
  options.fAvoidStencilBuffers = true;

  if (!createContext) {
    context_ = nullptr;
    return;
  }

  auto context = GrContext::MakeGL(GrGLMakeNativeInterface(), options);

  if (context == nullptr) {
    FXL_LOG(ERROR) << "Failed to setup Skia Gr context.";
    return;
  }

  context_ = std::move(context);

  context_->setResourceCacheLimits(kGrCacheMaxCount,
                                   kGrCacheMaxByteSize);
}

GpuGrContext::~GpuGrContext() {
  if (context_) {
    context_->releaseResourcesAndAbandonContext();
    context_ = nullptr;
  }
}

GrContext *GpuGrContext::GetContext() {
  return context_.get();
}

} // namespace shell
