// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/native_widget_layer.h"

#include "flutter/flow/texture.h"

#include "flutter/flow/system_compositor_context.h"

namespace flow {

NativeWidgetLayer::NativeWidgetLayer() = default;

NativeWidgetLayer::~NativeWidgetLayer() = default;

void NativeWidgetLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  set_paint_bounds(SkRect::MakeXYWH(
    offset_.x(),
    offset_.y(),
    size_.width(),
    size_.height()));
  set_needs_system_composite(true);
}

void NativeWidgetLayer::UpdateScene(SystemCompositorContext& context) {
  std::shared_ptr<Texture> texture =
      context.texture_registry->GetTexture(texture_id_);
  if (!texture) {
    return;
  }
  texture->UpdateScene(&context, paint_bounds());
}

void NativeWidgetLayer::Paint(PaintContext& context) const {
  FXL_NOTREACHED() << "This layer never needs painting.";
}

}  // namespace flow
