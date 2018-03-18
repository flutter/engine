// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_native_widget.h"

#include "flutter/flow/layers/layer.h"
#include "flutter/flow/system_compositor_context.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"

namespace shell {

IOSNativeWidget::IOSNativeWidget(int64_t textureId, UIView* view)
    : Texture(textureId), external_view_(view) {
  FXL_DCHECK(external_view_);
}

IOSNativeWidget::~IOSNativeWidget() = default;

void IOSNativeWidget::UpdateScene(flow::SystemCompositorContext* context,
                                          const SkRect& bounds) {
  ASSERT_IS_GPU_THREAD;
  context->AddNativeWidget(this, bounds);
}

void IOSNativeWidget::Paint(SkCanvas& canvas, const SkRect& bounds) {
  ASSERT_IS_GPU_THREAD;
}

void IOSNativeWidget::OnGrContextCreated() {
  ASSERT_IS_GPU_THREAD
}

void IOSNativeWidget::OnGrContextDestroyed() {
  ASSERT_IS_GPU_THREAD
}

}  // namespace shell
