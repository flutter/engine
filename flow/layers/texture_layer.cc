// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/texture_layer.h"

#include "flutter/flow/texture.h"

namespace flow {

TextureLayer::TextureLayer() = default;

TextureLayer::~TextureLayer() = default;

void TextureLayer::Preroll(PrerollContext* context,
                           const SkMatrix& matrix,
                           const SkIRect& device_clip) {
  set_paint_bounds(SkRect::MakeXYWH(offset_.x(), offset_.y(), size_.width(),
                                    size_.height()));

  device_paint_bounds_ = ComputeDeviceIRect(matrix, paint_bounds());
  if (!device_paint_bounds_.intersect(device_clip)) {
    device_paint_bounds_.setEmpty();
  }
}

void TextureLayer::Paint(PaintContext& context) const {
  std::shared_ptr<Texture> texture =
      context.texture_registry.GetTexture(texture_id_);
  if (!texture) {
    return;
  }
  texture->Paint(context.canvas, paint_bounds());
}

}  // namespace flow
