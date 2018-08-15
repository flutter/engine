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
  texture->Paint(context.canvas, paint_bounds(), freeze_);
}

// |fml::MessageSerializable|
bool TextureLayer::Serialize(fml::Message& message) const {
  FML_SERIALIZE(message, offset_);
  FML_SERIALIZE(message, size_);
  FML_SERIALIZE(message, texture_id_);
  FML_SERIALIZE(message, freeze_);
  return true;
}

// |fml::MessageSerializable|
bool TextureLayer::Deserialize(fml::Message& message) {
  FML_DESERIALIZE(message, offset_);
  FML_DESERIALIZE(message, size_);
  FML_DESERIALIZE(message, texture_id_);
  FML_DESERIALIZE(message, freeze_);
  return true;
}

}  // namespace flow
