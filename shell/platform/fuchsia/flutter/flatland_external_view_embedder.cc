// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flatland_external_view_embedder.h"

namespace flutter_runner {

FlatlandExternalViewEmbedder::FlatlandExternalViewEmbedder(
    std::string debug_label,
    fuchsia::ui::views::ViewToken view_token,
    scenic::ViewRefPair view_ref_pair,
    FlatlandConnection& session,
    VulkanSurfaceProducer& surface_producer,
    bool intercept_all_input) {}

FlatlandExternalViewEmbedder::~FlatlandExternalViewEmbedder() = default;

SkCanvas* FlatlandExternalViewEmbedder::GetRootCanvas() {
  return nullptr;
}

std::vector<SkCanvas*> FlatlandExternalViewEmbedder::GetCurrentCanvases() {
  return std::vector<SkCanvas*>();
}

void FlatlandExternalViewEmbedder::PrerollCompositeEmbeddedView(
    int view_id,
    std::unique_ptr<flutter::EmbeddedViewParams> params) {}

SkCanvas* FlatlandExternalViewEmbedder::CompositeEmbeddedView(int view_id) {
  return nullptr;
}

flutter::PostPrerollResult FlatlandExternalViewEmbedder::PostPrerollAction(
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  return flutter::PostPrerollResult::kSuccess;
}

void FlatlandExternalViewEmbedder::BeginFrame(
    SkISize frame_size,
    GrDirectContext* context,
    double device_pixel_ratio,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {}

void FlatlandExternalViewEmbedder::EndFrame(
    bool should_resubmit_frame,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {}

void FlatlandExternalViewEmbedder::SubmitFrame(
    GrDirectContext* context,
    std::unique_ptr<flutter::SurfaceFrame> frame,
    const std::shared_ptr<const fml::SyncSwitch>& gpu_disable_sync_switch) {}

void FlatlandExternalViewEmbedder::CancelFrame() {}

bool FlatlandExternalViewEmbedder::SupportsDynamicThreadMerging() {
  return false;
}

}  // namespace flutter_runner
