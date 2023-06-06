// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/rasterize_agent_surface.h"

namespace flutter {

RasterizeAgentSurface::RasterizeAgentSurface(std::shared_ptr<Surface> surface)
    : surface_(surface) {}

RasterizeAgentSurface::~RasterizeAgentSurface() = default;

void RasterizeAgentSurface::SubmitFrame(
    std::unique_ptr<SurfaceFrame> frame,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  frame->Submit();
}

PostPrerollResult RasterizeAgentSurface::PostPrerollAction(
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  return PostPrerollResult::kSuccess;
}

}  // namespace flutter
