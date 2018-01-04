// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/null_rasterizer.h"

namespace shell {

NullRasterizer::NullRasterizer(blink::TaskRunners task_runners)
    : Rasterizer(std::move(task_runners)) {}

void NullRasterizer::Setup(std::unique_ptr<Surface> surface_or_null) {
  surface_ = std::move(surface_or_null);
}

void NullRasterizer::Teardown() {
  surface_.reset();
}

flow::LayerTree* NullRasterizer::GetLastLayerTree() {
  return nullptr;
}

void NullRasterizer::DrawLastLayerTree() {
  // Null rasterizer. Nothing to do.
}

flow::TextureRegistry& NullRasterizer::GetTextureRegistry() {
  return *texture_registry_;
}

void NullRasterizer::Clear(SkColor color, const SkISize& size) {
  // Null rasterizer. Nothing to do.
}

void NullRasterizer::Draw(
    fxl::RefPtr<flutter::Pipeline<flow::LayerTree>> pipeline) {
  FXL_ALLOW_UNUSED_LOCAL(
      pipeline->Consume([](std::unique_ptr<flow::LayerTree>) {
        // Drop the layer tree on the floor. We only need the pipeline empty so
        // that frame requests are not deferred indefinitely due to
        // backpressure.
      }));
}

void NullRasterizer::AddNextFrameCallback(fxl::Closure nextFrameCallback) {
  // Null rasterizer. Nothing to do.
}

}  // namespace shell
