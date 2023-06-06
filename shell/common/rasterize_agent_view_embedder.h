// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_RASTERIZE_AGENT_VIEW_EMBEDDER_H_
#define SHELL_COMMON_RASTERIZE_AGENT_VIEW_EMBEDDER_H_

#include "flutter/flow/rasterize_agent.h"
#include "flutter/flow/surface.h"

namespace flutter {

class RasterizeAgentViewEmbedder : public RasterizeAgent {
 public:
  RasterizeAgentViewEmbedder(
      std::shared_ptr<ExternalViewEmbedder> view_embedder,
      std::shared_ptr<Surface> surface);

  ~RasterizeAgentViewEmbedder();

  void PreTearDown() override;

  bool SupportsDynamicThreadMerging() override {
    return view_embedder->SupportsDynamicThreadMerging();
  }

  bool AllowsPartialRepaint() override {
    // Disable partial repaint if ExternalViewEmbedder::SubmitFrame is
    // involved - ExternalViewEmbedder unconditionally clears the entire
    // surface and also partial repaint with platform view present is
    // something that still need to be figured out.
    return false;
  }

  DlCanvas* OnBeginFrame(
      const SkISize& frame_size,
      float pixel_ratio,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  void OnEndFrame(
      bool should_resubmit_frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  void SubmitFrame(
      std::unique_ptr<SurfaceFrame> frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  PostPrerollResult PostPrerollAction(
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  ExternalViewEmbedder* ViewEmbedder() override;

 private:
  std::shared_ptr<ExternalViewEmbedder> view_embedder_;
  std::shared_ptr<Surface> surface_;
  bool supports_thread_merging_;

  FML_DISALLOW_COPY_AND_ASSIGN(RasterizeAgentViewEmbedder);
};

}  // namespace flutter

#endif  // SHELL_COMMON_RASTERIZE_AGENT_VIEW_EMBEDDER_H_
