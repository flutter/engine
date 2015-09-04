// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/picture_rasterizer.h"

#include "sky/compositor/picture_state.h"
#include "sky/compositor/picture_table.h"

namespace sky {

PictureRasterizer::PictureRasterizer(GrContext* context,
                                     PictureTable* picture_table)
  : context_(context), picture_table_(picture_table) {
}

PictureRasterizer::~PictureRasterizer() {
}

void PictureRasterizer::Visit(PictureLayer* layer) {
  PictureState* state = picture_table_->GetAndMark(layer->picture());

  if (state->cached_image()) {
    layer->set_cached_image(state->cached_image());
    return;
  }

  if (!state->ShouldRasterize())
    return;

  GrTextureProvider* texture_provider = context_->textureProvider();

  GrSurfaceDesc desc;
  desc.fFlags = kRenderTarget_GrSurfaceFlag;
  desc.fWidth = 300;
  desc.fHeight = 400;

  GrTexture* texture = adoptRef(texture_provider->createTexture(desc, false));
}

void PictureRasterizer::Rasterize() {
}

}  // namespace sky
