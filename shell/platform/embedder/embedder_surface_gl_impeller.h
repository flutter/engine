// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_GL_IMPELLER_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_GL_IMPELLER_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"
#include "flutter/shell/platform/embedder/embedder_surface_gl_skia.h"
#include "flutter/shell/surface/surface_gl_impeller.h"

namespace impeller {
class ContextGLES;
}  // namespace impeller

namespace flutter {

class ReactorWorker;

class EmbedderSurfaceGLImpeller final : public EmbedderSurface,
                                        public SurfaceGLDelegate {
 public:
  EmbedderSurfaceGLImpeller(
      EmbedderSurfaceGLSkia::GLDispatchTable gl_dispatch_table,
      bool fbo_reset_after_present,
      std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder);

  ~EmbedderSurfaceGLImpeller() override;

 private:
  bool valid_ = false;
  EmbedderSurfaceGLSkia::GLDispatchTable gl_dispatch_table_;
  bool fbo_reset_after_present_;
  std::shared_ptr<impeller::ContextGLES> impeller_context_;
  std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder_;
  std::shared_ptr<ReactorWorker> worker_;

  // |EmbedderSurface|
  bool IsValid() const override;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  // |EmbedderSurface|
  std::shared_ptr<impeller::Context> CreateImpellerContext() const override;

  // |SurfaceGLDelegate|
  std::unique_ptr<GLContextResult> GLContextMakeCurrent() override;

  // |SurfaceGLDelegate|
  bool GLContextClearCurrent() override;

  // |SurfaceGLDelegate|
  bool GLContextPresent(const GLPresentInfo& present_info) override;

  // |SurfaceGLDelegate|
  GLFBOInfo GLContextFBO(GLFrameInfo frame_info) const override;

  // |SurfaceGLDelegate|
  bool GLContextFBOResetAfterPresent() const override;

  // |SurfaceGLDelegate|
  SkMatrix GLContextSurfaceTransformation() const override;

  // |SurfaceGLDelegate|
  GLProcResolver GetGLProcResolver() const override;

  // |SurfaceGLDelegate|
  SurfaceFrame::FramebufferInfo GLContextFramebufferInfo() const override;

  // |EmbedderSurface|
  sk_sp<GrDirectContext> CreateResourceContext() const override;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceGLImpeller);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_GL_IMPELLER_H_
