// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/texture_layer.h"

#include "flutter/flow/texture.h"

namespace flow {

TextureLayer::TextureLayer() = default;

TextureLayer::~TextureLayer() = default;

void TextureLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  set_paint_bounds(SkRect::MakeXYWH(offset_.x(), offset_.y(), size_.width(),
                                    size_.height()));
}

void TextureLayer::Paint(PaintContext& context) const {
  std::shared_ptr<Texture> texture =
      context.texture_registry.GetTexture(texture_id_);
  if (!texture) {
    return;
  }
  sk_sp<SkImage> sk_image =
      texture->MakeSkImage(paint_bounds().width(), paint_bounds().height(),
                           context.canvas.getGrContext());
  if (!sk_image) {
    return;
  }
  context.canvas.drawImage(sk_image, paint_bounds().x(), paint_bounds().y());
}

}  // namespace flow
