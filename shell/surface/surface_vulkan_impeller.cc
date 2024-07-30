// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/surface/surface_vulkan_impeller.h"

#include "flutter/fml/make_copyable.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/renderer/backend/vulkan/surface_context_vk.h"
#include "impeller/renderer/surface.h"
#include "impeller/typographer/backends/skia/typographer_context_skia.h"

namespace flutter {

SurfaceVulkanImpeller::SurfaceVulkanImpeller(
    std::shared_ptr<impeller::Context> context) {
  if (!context || !context->IsValid()) {
    return;
  }

  auto aiks_context = std::make_shared<impeller::AiksContext>(
      context, impeller::TypographerContextSkia::Make());
  if (!aiks_context->IsValid()) {
    return;
  }

  impeller_context_ = std::move(context);
  aiks_context_ = std::move(aiks_context);
  is_valid_ = true;
}

// |Surface|
SurfaceVulkanImpeller::~SurfaceVulkanImpeller() = default;

// |Surface|
bool SurfaceVulkanImpeller::IsValid() {
  return is_valid_;
}

// |Surface|
std::unique_ptr<SurfaceFrame> SurfaceVulkanImpeller::AcquireFrame(
    const SkISize& size) {
  if (!IsValid()) {
    FML_LOG(ERROR) << "Vulkan surface was invalid.";
    return nullptr;
  }

  if (size.isEmpty()) {
    FML_LOG(ERROR) << "Vulkan surface was asked for an empty frame.";
    return nullptr;
  }

  auto& context_vk = impeller::SurfaceContextVK::Cast(*impeller_context_);
  std::unique_ptr<impeller::Surface> surface = context_vk.AcquireNextSurface();

  if (!surface) {
    FML_LOG(ERROR) << "No surface available.";
    return nullptr;
  }

  SurfaceFrame::SubmitCallback submit_callback =
      fml::MakeCopyable([aiks_context = aiks_context_,  //
                         surface = std::move(surface)   //
  ](SurfaceFrame& surface_frame, DlCanvas* canvas) mutable -> bool {
        if (!aiks_context) {
          return false;
        }

        auto display_list = surface_frame.BuildDisplayList();
        if (!display_list) {
          FML_LOG(ERROR) << "Could not build display list for surface frame.";
          return false;
        }

        auto cull_rect =
            surface->GetTargetRenderPassDescriptor().GetRenderTargetSize();

        const impeller::RenderTarget& render_target =
            surface->GetTargetRenderPassDescriptor();
#if EXPERIMENTAL_CANVAS
        impeller::TextFrameDispatcher collector(
            aiks_context->GetContentContext(), impeller::Matrix());
        display_list->Dispatch(
            collector, SkIRect::MakeWH(cull_rect.width, cull_rect.height));
        impeller::ExperimentalDlDispatcher impeller_dispatcher(
            aiks_context->GetContentContext(), render_target,
            display_list->root_has_backdrop_filter(),
            display_list->max_root_blend_mode(),
            impeller::IRect::RoundOut(impeller::Rect::MakeSize(cull_rect)));
        display_list->Dispatch(
            impeller_dispatcher,
            SkIRect::MakeWH(cull_rect.width, cull_rect.height));
        impeller_dispatcher.FinishRecording();
        aiks_context->GetContentContext().GetTransientsBuffer().Reset();
        aiks_context->GetContentContext()
            .GetLazyGlyphAtlas()
            ->ResetTextFrames();
        return surface->Present();
#else
        impeller::Rect dl_cull_rect = impeller::Rect::MakeSize(cull_rect);
        impeller::DlDispatcher impeller_dispatcher(dl_cull_rect);
        display_list->Dispatch(
            impeller_dispatcher,
            SkIRect::MakeWH(cull_rect.width, cull_rect.height));
        auto picture = impeller_dispatcher.EndRecordingAsPicture();
        const bool reset_host_buffer =
            surface_frame.submit_info().frame_boundary;
        if (!aiks_context->Render(picture, render_target, reset_host_buffer)) {
          return false;
        }
        return surface->Present();
#endif
      });

  return std::make_unique<SurfaceFrame>(
      nullptr,                          // surface
      SurfaceFrame::FramebufferInfo{},  // framebuffer info
      submit_callback,                  // submit callback
      size,                             // frame size
      nullptr,                          // context result
      true                              // display list fallback
  );
}

// |Surface|
SkMatrix SurfaceVulkanImpeller::GetRootTransformation() const {
  // This backend does not currently support root surface transformations. Just
  // return identity.
  return {};
}

// |Surface|
GrDirectContext* SurfaceVulkanImpeller::GetContext() {
  // Impeller != Skia.
  return nullptr;
}

// |Surface|
std::unique_ptr<GLContextResult>
SurfaceVulkanImpeller::MakeRenderContextCurrent() {
  // This backend has no such concept.
  return std::make_unique<GLContextDefaultResult>(true);
}

// |Surface|
bool SurfaceVulkanImpeller::EnableRasterCache() const {
  return false;
}

// |Surface|
std::shared_ptr<impeller::AiksContext> SurfaceVulkanImpeller::GetAiksContext()
    const {
  return aiks_context_;
}

}  // namespace flutter
