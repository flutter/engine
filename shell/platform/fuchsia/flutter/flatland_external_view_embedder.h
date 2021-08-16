// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_FLATLAND_EXTERNAL_VIEW_EMBEDDER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_FLATLAND_EXTERNAL_VIEW_EMBEDDER_H_

#include <fuchsia/ui/composition/cpp/fidl.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>

#include "flutter/flow/embedded_views.h"
#include "flutter/fml/macros.h"

#include "flatland_connection.h"
#include "vulkan_surface_producer.h"

namespace flutter_runner {

// This class orchestrates interaction with the Scenic's Flatland compositor on
// Fuchsia. It ensures that flutter content and platform view content are both
// rendered correctly in a unified scene.
class FlatlandExternalViewEmbedder final
    : public flutter::ExternalViewEmbedder {
 public:
  FlatlandExternalViewEmbedder(std::string debug_label,
                               fuchsia::ui::views::ViewToken view_token,
                               scenic::ViewRefPair view_ref_pair,
                               FlatlandConnection& session,
                               VulkanSurfaceProducer& surface_producer,
                               bool intercept_all_input = false);
  ~FlatlandExternalViewEmbedder();

  // |ExternalViewEmbedder|
  SkCanvas* GetRootCanvas() override;

  // |ExternalViewEmbedder|
  std::vector<SkCanvas*> GetCurrentCanvases() override;

  // |ExternalViewEmbedder|
  void PrerollCompositeEmbeddedView(
      int view_id,
      std::unique_ptr<flutter::EmbeddedViewParams> params) override;

  // |ExternalViewEmbedder|
  SkCanvas* CompositeEmbeddedView(int view_id) override;

  // |ExternalViewEmbedder|
  flutter::PostPrerollResult PostPrerollAction(
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  // |ExternalViewEmbedder|
  void BeginFrame(
      SkISize frame_size,
      GrDirectContext* context,
      double device_pixel_ratio,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  // |ExternalViewEmbedder|
  void EndFrame(
      bool should_resubmit_frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  // |ExternalViewEmbedder|
  void SubmitFrame(GrDirectContext* context,
                   std::unique_ptr<flutter::SurfaceFrame> frame,
                   const std::shared_ptr<const fml::SyncSwitch>&
                       gpu_disable_sync_switch) override;

  // |ExternalViewEmbedder|
  void CancelFrame() override;

  // |ExternalViewEmbedder|
  bool SupportsDynamicThreadMerging() override;

  FML_DISALLOW_COPY_AND_ASSIGN(FlatlandExternalViewEmbedder);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_FLATLAND_EXTERNAL_VIEW_EMBEDDER_H_
