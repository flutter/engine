// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_RASTERIZE_AGENT_SURFACE_H_
#define SHELL_COMMON_RASTERIZE_AGENT_SURFACE_H_

#include "flutter/flow/rasterize_agent.h"
#include "flutter/flow/surface.h"

namespace flutter {

class RasterizeAgentSurface : public RasterizeAgent {
 public:
  explicit RasterizeAgentSurface(std::shared_ptr<Surface> surface);

  ~RasterizeAgentSurface();

  void PreTearDown() override {}

  bool SupportsDynamicThreadMerging() override { return false; }

  bool AllowsPartialRepaint() override { return true; }

  DlCanvas* OnBeginFrame(
      const SkISize& frame_size,
      float pixel_ratio,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override {
    return nullptr;
  }

  void OnEndFrame(
      bool should_resubmit_frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override {}

  void SubmitFrame(
      std::unique_ptr<SurfaceFrame> frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  PostPrerollResult PostPrerollAction(
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override;

  ExternalViewEmbedder* ViewEmbedder() override { return nullptr; }

 private:
  std::shared_ptr<Surface> surface_;

  FML_DISALLOW_COPY_AND_ASSIGN(RasterizeAgentSurface);
};

}  // namespace flutter

#endif  // SHELL_COMMON_RASTERIZE_AGENT_SURFACE_H_
