// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_SURFACE_GL_H_
#define SHELL_GPU_GPU_SURFACE_GL_H_

#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/gpu/gpu_gr_context.h"
#include "flutter/shell/common/surface.h"
#include "flutter/synchronization/debug_thread_checker.h"
#include "lib/fxl/macros.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace shell {

class GPUSurfaceGLDelegate {
 public:
  virtual bool GLContextMakeCurrent() = 0;

  virtual bool GLContextClearCurrent() = 0;

  virtual bool GLContextPresent() = 0;

  virtual intptr_t GLContextFBO() const = 0;

  virtual bool UseOffscreenSurface() const { return false; }
};

class GPUSurfaceGL : public Surface {
 public:
  GPUSurfaceGL(GPUSurfaceGLDelegate* delegate, GrContext *grContext);

  ~GPUSurfaceGL() override;

  bool IsValid() override;

  std::unique_ptr<SurfaceFrame> AcquireFrame(const SkISize& size) override;

  GrContext* GetContext() override;

  sk_sp<SkSurface> AcquireRenderSurface(const SkISize& size);
  bool PointIsTransparent(SkPoint point);

  bool PresentSurface(SkCanvas* canvas);
  bool PresentSurface();

  bool MakeCurrent();

 private:
  GPUSurfaceGLDelegate* delegate_;
  GrContext *gr_context_;
  sk_sp<SkSurface> onscreen_surface_;
  sk_sp<SkSurface> offscreen_surface_;
  sk_sp<SkImage> saved_image_;
  bool valid_ = false;
  fml::WeakPtrFactory<GPUSurfaceGL> weak_factory_;

  bool CreateOrUpdateSurfaces(const SkISize& size);

  bool SelectPixelConfig(GrPixelConfig* config);

  FXL_DISALLOW_COPY_AND_ASSIGN(GPUSurfaceGL);
};

}  // namespace shell

#endif  // SHELL_GPU_GPU_SURFACE_GL_H_
