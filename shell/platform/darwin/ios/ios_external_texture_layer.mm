// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_external_texture_layer.h"

#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/layered_paint_context.h"

namespace shell {

IOSExternalTextureLayer::IOSExternalTextureLayer(int64_t textureId,
                                                 CALayer *externalLayer)
    : Texture(textureId), external_layer_(externalLayer) {
  FXL_DCHECK(external_layer_);
}

IOSExternalTextureLayer::~IOSExternalTextureLayer() = default;

void IOSExternalTextureLayer::UpdateScene(flow::LayeredPaintContext *context, const SkRect& bounds) {
  ASSERT_IS_GPU_THREAD;
  context->AddExternalLayer(this, bounds);
}

void IOSExternalTextureLayer::Paint(flow::Layer::PaintContext context, const SkRect& bounds) {
  ASSERT_IS_GPU_THREAD;
}

void IOSExternalTextureLayer::OnGrContextCreated() {
  ASSERT_IS_GPU_THREAD
}

void IOSExternalTextureLayer::OnGrContextDestroyed() {
  ASSERT_IS_GPU_THREAD
}

}  // namespace shell
