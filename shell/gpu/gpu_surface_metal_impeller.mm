// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_metal_impeller.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include "flutter/common/settings.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/mapping.h"
#include "flutter/fml/trace_event.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/renderer/backend/metal/surface_mtl.h"
#include "impeller/typographer/backends/skia/typographer_context_skia.h"

static_assert(!__has_feature(objc_arc), "ARC must be disabled.");

namespace flutter {

static std::shared_ptr<impeller::Renderer> CreateImpellerRenderer(
    std::shared_ptr<impeller::Context> context) {
  auto renderer = std::make_shared<impeller::Renderer>(std::move(context));
  if (!renderer->IsValid()) {
    FML_LOG(ERROR) << "Could not create valid Impeller Renderer.";
    return nullptr;
  }
  return renderer;
}

GPUSurfaceMetalImpeller::GPUSurfaceMetalImpeller(GPUSurfaceMetalDelegate* delegate,
                                                 const std::shared_ptr<impeller::Context>& context,
                                                 bool render_to_surface)
    : delegate_(delegate),
      render_target_type_(delegate->GetRenderTargetType()),
      impeller_renderer_(CreateImpellerRenderer(context)),
      aiks_context_(
          std::make_shared<impeller::AiksContext>(impeller_renderer_ ? context : nullptr,
                                                  impeller::TypographerContextSkia::Make())),
      render_to_surface_(render_to_surface) {}

GPUSurfaceMetalImpeller::~GPUSurfaceMetalImpeller() = default;

// |Surface|
bool GPUSurfaceMetalImpeller::IsValid() {
  return !!aiks_context_ && aiks_context_->IsValid();
}

// |Surface|
std::unique_ptr<SurfaceFrame> GPUSurfaceMetalImpeller::AcquireFrame(const SkISize& frame_size) {
  TRACE_EVENT0("impeller", "GPUSurfaceMetalImpeller::AcquireFrame");

  if (!IsValid()) {
    FML_LOG(ERROR) << "Metal surface was invalid.";
    return nullptr;
  }

  if (frame_size.isEmpty()) {
    FML_LOG(ERROR) << "Metal surface was asked for an empty frame.";
    return nullptr;
  }

  if (!render_to_surface_) {
    return std::make_unique<SurfaceFrame>(
        nullptr, SurfaceFrame::FramebufferInfo(),
        [](const SurfaceFrame& surface_frame, DlCanvas* canvas) { return true; }, frame_size);
  }

  switch (render_target_type_) {
    case MTLRenderTargetType::kCAMetalLayer:
      return AcquireFrameFromCAMetalLayer(frame_size);
    case MTLRenderTargetType::kMTLTexture:
      return AcquireFrameFromMTLTexture(frame_size);
    default:
      FML_CHECK(false) << "Unknown MTLRenderTargetType type.";
  }

  return nullptr;
}

std::unique_ptr<SurfaceFrame> GPUSurfaceMetalImpeller::AcquireFrameFromCAMetalLayer(
    const SkISize& frame_size) {
  auto layer = delegate_->GetCAMetalLayer(frame_size);

  if (!layer) {
    FML_LOG(ERROR) << "Invalid CAMetalLayer given by the embedder.";
    return nullptr;
  }

  auto* mtl_layer = (CAMetalLayer*)layer;

  SurfaceFrame::SubmitCallback submit_callback =
      fml::MakeCopyable([this,                           //
                         renderer = impeller_renderer_,  //
                         aiks_context = aiks_context_,   //
                         mtl_layer                       //
  ](SurfaceFrame& surface_frame, DlCanvas* canvas) mutable -> bool {
        if (!aiks_context) {
          return false;
        }

        auto display_list = surface_frame.BuildDisplayList();
        if (!display_list) {
          FML_LOG(ERROR) << "Could not build display list for surface frame.";
          return false;
        }

        auto surface =
            impeller::SurfaceMTL::MakeFromMetalLayer(impeller_renderer_->GetContext(), mtl_layer);

        impeller::IRect cull_rect = surface->coverage();
        SkIRect sk_cull_rect = SkIRect::MakeWH(cull_rect.size.width, cull_rect.size.height);
        impeller::DlDispatcher impeller_dispatcher(cull_rect);
        display_list->Dispatch(impeller_dispatcher, sk_cull_rect);
        auto picture = impeller_dispatcher.EndRecordingAsPicture();

        return renderer->Render(
            std::move(surface),
            fml::MakeCopyable([aiks_context, picture = std::move(picture)](
                                  impeller::RenderTarget& render_target) -> bool {
              return aiks_context->Render(picture, render_target);
            }));
      });

  SurfaceFrame::FramebufferInfo framebuffer_info;

  return std::make_unique<SurfaceFrame>(nullptr,           // surface
                                        framebuffer_info,  // framebuffer info
                                        submit_callback,   // submit callback
                                        frame_size,        // frame size
                                        nullptr,           // context result
                                        true               // display list fallback
  );
}

std::unique_ptr<SurfaceFrame> GPUSurfaceMetalImpeller::AcquireFrameFromMTLTexture(
    const SkISize& frame_size) {
  GPUMTLTextureInfo texture_info = delegate_->GetMTLTexture(frame_size);
  id<MTLTexture> mtl_texture = (id<MTLTexture>)(texture_info.texture);

  if (!mtl_texture) {
    FML_LOG(ERROR) << "Invalid MTLTexture given by the embedder.";
    return nullptr;
  }

  SurfaceFrame::SubmitCallback submit_callback =
      fml::MakeCopyable([                                   //
                            renderer = impeller_renderer_,  //
                            aiks_context = aiks_context_    //           //
  ](SurfaceFrame& surface_frame, DlCanvas* canvas) mutable -> bool {
        if (!aiks_context) {
          return false;
        }

        auto display_list = surface_frame.BuildDisplayList();
        if (!display_list) {
          FML_LOG(ERROR) << "Could not build display list for surface frame.";
          return false;
        }
        return false;

        // auto surface =
        //     impeller::SurfaceMTL::MakeFromTexture(renderer->GetContext(), mtl_texture);

        // impeller::IRect cull_rect = surface->coverage();
        // SkIRect sk_cull_rect = SkIRect::MakeWH(cull_rect.size.width, cull_rect.size.height);
        // impeller::DlDispatcher impeller_dispatcher(cull_rect);
        // display_list->Dispatch(impeller_dispatcher, sk_cull_rect);
        // auto picture = impeller_dispatcher.EndRecordingAsPicture();

        // bool render_result =
        //     renderer->Render(std::move(surface),
        //                      fml::MakeCopyable([aiks_context, picture = std::move(picture)](
        //                                            impeller::RenderTarget& render_target) -> bool
        //                                            {
        //                        return aiks_context->Render(picture, render_target);
        //                      }));
        // if (!render_result) {
        //   FML_LOG(ERROR) << "Failed to render Impeller frame";
        //   return false;
        // }

        // return delegate->PresentTexture(texture_info);
      });

  SurfaceFrame::FramebufferInfo framebuffer_info;
  framebuffer_info.supports_readback = false;

  return std::make_unique<SurfaceFrame>(nullptr,           // surface
                                        framebuffer_info,  // framebuffer info
                                        submit_callback,   // submit callback
                                        frame_size,        // frame size
                                        nullptr,           // context result
                                        true               // display list fallback
  );
}

// |Surface|
SkMatrix GPUSurfaceMetalImpeller::GetRootTransformation() const {
  // This backend does not currently support root surface transformations. Just
  // return identity.
  return {};
}

// |Surface|
GrDirectContext* GPUSurfaceMetalImpeller::GetContext() {
  return nullptr;
}

// |Surface|
std::unique_ptr<GLContextResult> GPUSurfaceMetalImpeller::MakeRenderContextCurrent() {
  // This backend has no such concept.
  return std::make_unique<GLContextDefaultResult>(true);
}

bool GPUSurfaceMetalImpeller::AllowsDrawingWhenGpuDisabled() const {
  return delegate_->AllowsDrawingWhenGpuDisabled();
}

// |Surface|
bool GPUSurfaceMetalImpeller::EnableRasterCache() const {
  return false;
}

// |Surface|
std::shared_ptr<impeller::AiksContext> GPUSurfaceMetalImpeller::GetAiksContext() const {
  return aiks_context_;
}

Surface::SurfaceData GPUSurfaceMetalImpeller::GetSurfaceData() const {
  return {};
}

}  // namespace flutter
