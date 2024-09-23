// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/interop/surface.h"

#include "impeller/aiks/aiks_context.h"
#include "impeller/base/validation.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/renderer/backend/gles/surface_gles.h"
#include "impeller/toolkit/interop/formats.h"

namespace impeller::interop {

Surface::Surface(Context& context, std::shared_ptr<impeller::Surface> surface)
    : context_(Ref(&context)), surface_(std::move(surface)) {
  is_valid_ =
      context_ && context_->IsValid() && surface_ && surface_->IsValid();
}

Surface::~Surface() = default;

ScopedObject<Surface> Surface::WrapFBO(Context& context,
                                       uint64_t fbo,
                                       PixelFormat color_format,
                                       ISize size) {
  if (context.GetContext()->GetBackendType() !=
      impeller::Context::BackendType::kOpenGLES) {
    VALIDATION_LOG << "Context is not OpenGL ES based.";
    return nullptr;
  }

  auto impeller_surface = impeller::SurfaceGLES::WrapFBO(
      context.GetContext(), []() { return true; }, fbo, color_format, size);
  if (!impeller_surface || !impeller_surface->IsValid()) {
    VALIDATION_LOG << "Could not wrap FBO as a surface";
    return nullptr;
  }

  auto surface = Create<Surface>(context, std::move(impeller_surface));
  if (!surface->IsValid()) {
    VALIDATION_LOG << "Could not create valid surface.";
    return nullptr;
  }
  return surface;
}

bool Surface::IsValid() const {
  return is_valid_;
}

bool Surface::DrawDisplayList(const DisplayList& dl) const {
  if (!IsValid() || !dl.IsValid()) {
    return false;
  }

  auto display_list = dl.GetDisplayList();
  auto& content_context = context_->GetAiksContext().GetContentContext();
  auto render_target = surface_->GetTargetRenderPassDescriptor();

  const auto cull_rect = IRect::MakeSize(surface_->GetSize());
  auto skia_cull_rect =
      SkIRect::MakeWH(cull_rect.GetWidth(), cull_rect.GetHeight());
  impeller::TextFrameDispatcher collector(content_context, impeller::Matrix{});
  display_list->Dispatch(collector, skia_cull_rect);

  impeller::ExperimentalDlDispatcher impeller_dispatcher(
      content_context,                           //
      render_target,                             //
      display_list->root_has_backdrop_filter(),  //
      display_list->max_root_blend_mode(),       //
      cull_rect                                  //
  );
  display_list->Dispatch(impeller_dispatcher, skia_cull_rect);
  impeller_dispatcher.FinishRecording();
  content_context.GetLazyGlyphAtlas()->ResetTextFrames();
  content_context.GetTransientsBuffer().Reset();
  return true;
}

}  // namespace impeller::interop
