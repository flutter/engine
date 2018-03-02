// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/texture_layer.h"

#include "flutter/flow/texture.h"

#include "flutter/flow/system_compositor_context.h"

namespace flow {

TextureLayer::TextureLayer() = default;

TextureLayer::~TextureLayer() = default;

void TextureLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  set_paint_bounds(SkRect::MakeXYWH(offset_.x(), offset_.y(), size_.width(),
                                    size_.height()));
  std::shared_ptr<Texture> texture =
      context->texture_registry.GetTexture(texture_id_);
  if (!texture) {
    return;
  }
  set_needs_system_composite(texture->NeedsSystemComposite());
}

void TextureLayer::UpdateScene(SystemCompositorContext& context) {
  std::string s = "";
  for (int i = 0; i < indent; i++) {
    s += "  ";
  }

  FXL_DLOG(INFO) << s << "Texture";

  std::shared_ptr<Texture> texture =
      context.texture_registry->GetTexture(texture_id_);
  if (!texture) {
    return;
  }
  texture->UpdateScene(&context, paint_bounds());
}

void TextureLayer::Paint(PaintContext& context) const {
  std::shared_ptr<Texture> texture =
      context.texture_registry.GetTexture(texture_id_);
  if (!texture) {
    return;
  }
  texture->Paint(context, paint_bounds());
}

}  // namespace flow
