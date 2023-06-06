// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/rasterize_agent_view_embedder.h"

namespace flutter {

RasterizeAgentViewEmbedder::RasterizeAgentViewEmbedder(
    std::shared_ptr<ExternalViewEmbedder> view_embedder,
    std::shared_ptr<Surface> surface)
    : view_embedder_(std::move(view_embedder)), surface_(std::move(surface)) {
  FML_DCHECK(view_embedder_);
}

RasterizeAgentViewEmbedder::~RasterizeAgentViewEmbedder() = default;

void RasterizeAgentViewEmbedder::PreTearDown() {
  view_embedder_->Teardown();
}

bool RasterizeAgentViewEmbedder::SupportsDynamicThreadMerging() {
  return view_embedder_->SupportsDynamicThreadMerging();
}

DlCanvas* RasterizeAgentViewEmbedder::OnBeginFrame(
    const SkISize& frame_size,
    float pixel_ratio,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  FML_DCHECK(!view_embedder_->GetUsedThisFrame());
  view_embedder_->SetUsedThisFrame(true);
  view_embedder_->BeginFrame(frame_size, surface_->GetContext(), pixel_ratio,
                             raster_thread_merger);
  return view_embedder_->GetRootCanvas();
}

void RasterizeAgentViewEmbedder::OnEndFrame(
    bool should_resubmit_frame,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  if (view_embedder_->GetUsedThisFrame()) {
    view_embedder_->SetUsedThisFrame(false);
    view_embedder_->EndFrame(should_resubmit_frame, raster_thread_merger);
  }
}

void RasterizeAgentViewEmbedder::SubmitFrame(
    std::unique_ptr<SurfaceFrame> frame,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  if (!raster_thread_merger || raster_thread_merger->IsMerged()) {
    FML_DCHECK(!frame->IsSubmitted());
    view_embedder_->SubmitFrame(surface_->GetContext(),
                                surface_->GetAiksContext(), std::move(frame));
  } else {
    frame->Submit();
  }
}

PostPrerollResult RasterizeAgentViewEmbedder::PostPrerollAction(
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  return view_embedder_->PostPrerollAction(raster_thread_merger);
}

ExternalViewEmbedder* RasterizeAgentViewEmbedder::ViewEmbedder() {
  return view_embedder_.get();
}

}  // namespace flutter
