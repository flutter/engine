// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/testing/mock_embedder.h"

namespace flutter {
namespace testing {

MockViewEmbedder::MockViewEmbedder(SkCanvas* root_canvas)
    : root_canvas_(root_canvas) {}

SkCanvas* MockViewEmbedder::GetRootCanvas() {
  return root_canvas_;
}

void MockViewEmbedder::CancelFrame() {}

void MockViewEmbedder::BeginFrame(
    SkISize frame_size,
    GrDirectContext* context,
    double device_pixel_ratio,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {}

void MockViewEmbedder::PrerollCompositeEmbeddedView(
    int view_id,
    std::unique_ptr<EmbeddedViewParams> params) {}

std::vector<SkCanvas*> MockViewEmbedder::GetCurrentCanvases() {
  return std::vector<SkCanvas*>({root_canvas_});
}

SkCanvas* MockViewEmbedder::CompositeEmbeddedView(int view_id) {
  return root_canvas_;
}

}  // namespace testing
}  // namespace flutter
