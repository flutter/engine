// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_GL_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_GL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_gl_skia.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_studio.h"

namespace flutter {

class EmbedderStudioGL final : public EmbedderStudio,
                               public GPUSurfaceGLDelegate {
 public:
  struct GLDispatchTable {
    std::function<bool(void)> gl_make_current_callback;           // required
    std::function<bool(void)> gl_clear_current_callback;          // required
    std::function<bool(GLPresentInfo)> gl_present_callback;       // required
    std::function<intptr_t(GLFrameInfo)> gl_fbo_callback;         // required
    std::function<bool(void)> gl_make_resource_current_callback;  // optional
    std::function<SkMatrix(void)>
        gl_surface_transformation_callback;                          // optional
    std::function<void*(const char*)> gl_proc_resolver;              // optional
    std::function<GLFBOInfo(intptr_t)> gl_populate_existing_damage;  // required
  };

  EmbedderStudioGL(
      GLDispatchTable gl_dispatch_table,
      bool fbo_reset_after_present,
      std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder);

  ~EmbedderStudioGL() override;

  // |EmbedderStudio|
  bool IsValid() const override;

  // |EmbedderStudio|
  std::unique_ptr<Studio> CreateGPUStudio() override;

  // |EmbedderStudio|
  std::unique_ptr<EmbedderSurface> CreateSurface() override;

  // |EmbedderStudio|
  sk_sp<GrDirectContext> CreateResourceContext() const override;

  // |GPUSurfaceGLDelegate|
  std::unique_ptr<GLContextResult> GLContextMakeCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextClearCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextPresent(const GLPresentInfo& present_info) override;

  // |GPUSurfaceGLDelegate|
  GLFBOInfo GLContextFBO(GLFrameInfo frame_info) const override;

  // |GPUSurfaceGLDelegate|
  bool GLContextFBOResetAfterPresent() const override;

  // |GPUSurfaceGLDelegate|
  SkMatrix GLContextSurfaceTransformation() const override;

  // |GPUSurfaceGLDelegate|
  GLProcResolver GetGLProcResolver() const override;

  // |GPUSurfaceGLDelegate|
  SurfaceFrame::FramebufferInfo GLContextFramebufferInfo() const override;

 private:
  bool valid_ = false;
  GLDispatchTable gl_dispatch_table_;
  bool fbo_reset_after_present_;
  sk_sp<GrDirectContext> main_context_;

  std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder_;

  sk_sp<GrDirectContext> MainContext();

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderStudioGL);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_STUDIO_GL_H_
