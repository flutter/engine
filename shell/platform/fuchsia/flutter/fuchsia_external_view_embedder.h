// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_FUCHSIA_EXTERNAL_VIEW_EMBEDDER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_FUCHSIA_EXTERNAL_VIEW_EMBEDDER_H_

#include "flutter/flow/embedded_views.h"

namespace flutter_runner {

// This class orchestrates interaction with the Scenic compositor on Fuchsia. It
// ensures that flutter content and platform view content are both rendered
// correctly in a unified scene.
//
// This minimal implementation crashes immediately but is enough to verify
// compilation with all legacy code removed.
class FuchsiaExternalViewEmbedder final : public flutter::ExternalViewEmbedder {
 public:
  FuchsiaExternalViewEmbedder();
  ~FuchsiaExternalViewEmbedder();

  // |ExternalViewEmbedder|
  SkCanvas* GetRootCanvas() override;
  std::vector<SkCanvas*> GetCurrentCanvases() override;

  // |ExternalViewEmbedder|
  void PrerollCompositeEmbeddedView(
      int view_id,
      std::unique_ptr<flutter::EmbeddedViewParams> params) override;
  SkCanvas* CompositeEmbeddedView(int view_id) override;
  flutter::PostPrerollResult PostPrerollAction(
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  // |ExternalViewEmbedder|
  void BeginFrame(
      SkISize frame_size,
      GrDirectContext* context,
      double device_pixel_ratio,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;
  void EndFrame(
      bool should_resubmit_frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;
  void SubmitFrame(GrDirectContext* context,
                   std::unique_ptr<flutter::SurfaceFrame> frame) override;
  void CancelFrame() override;

  // |ExternalViewEmbedder|
  bool SupportsDynamicThreadMerging() override;

  // View manipulation.
  void EnableWireframe(bool enable);
  void CreateView(int64_t view_id, bool hit_testable, bool focusable);
  void UpdateView(int64_t view_id, bool hit_testable, bool focusable);
  void DestroyView(int64_t view_id);
  void UpdateView(int64_t view_id,
                  const SkPoint& offset,
                  const SkSize& size,
                  std::optional<bool> override_hit_testable = std::nullopt);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_FUCHSIA_EXTERNAL_VIEW_EMBEDDER_H_
