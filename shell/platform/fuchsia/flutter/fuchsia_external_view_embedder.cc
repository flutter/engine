// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fuchsia_external_view_embedder.h"

namespace flutter_runner {

FuchsiaExternalViewEmbedder::FuchsiaExternalViewEmbedder() = default;

FuchsiaExternalViewEmbedder::~FuchsiaExternalViewEmbedder() = default;

SkCanvas* FuchsiaExternalViewEmbedder::GetRootCanvas() {
  return nullptr;
}

std::vector<SkCanvas*> FuchsiaExternalViewEmbedder::GetCurrentCanvases() {
  return std::vector<SkCanvas*>();
}

void FuchsiaExternalViewEmbedder::PrerollCompositeEmbeddedView(
    int view_id,
    std::unique_ptr<flutter::EmbeddedViewParams> params) {}

SkCanvas* FuchsiaExternalViewEmbedder::CompositeEmbeddedView(int view_id) {
  return nullptr;
}

flutter::PostPrerollResult FuchsiaExternalViewEmbedder::PostPrerollAction(
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  return flutter::PostPrerollResult::kSuccess;
}

void FuchsiaExternalViewEmbedder::BeginFrame(
    SkISize frame_size,
    GrDirectContext* context,
    double device_pixel_ratio,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {}

void FuchsiaExternalViewEmbedder::EndFrame(
    bool should_resubmit_frame,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {}

void FuchsiaExternalViewEmbedder::SubmitFrame(
    GrDirectContext* context,
    std::unique_ptr<flutter::SurfaceFrame> frame) {}

void FuchsiaExternalViewEmbedder::CancelFrame() {}

bool FuchsiaExternalViewEmbedder::SupportsDynamicThreadMerging() {
  return false;
}

void FuchsiaExternalViewEmbedder::EnableWireframe(bool enable) {}

void FuchsiaExternalViewEmbedder::CreateView(int64_t view_id,
                                             bool hit_testable,
                                             bool focusable) {}

void FuchsiaExternalViewEmbedder::UpdateView(int64_t view_id,
                                             bool hit_testable,
                                             bool focusable) {}

void FuchsiaExternalViewEmbedder::DestroyView(int64_t view_id) {}

void FuchsiaExternalViewEmbedder::UpdateView(
    int64_t view_id,
    const SkPoint& offset,
    const SkSize& size,
    std::optional<bool> override_hit_testable) {}

}  // namespace flutter_runner
