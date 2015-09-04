// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/picture_rasterizer.h"

#include "sky/compositor/picture_state.h"
#include "sky/compositor/picture_table.h"

namespace sky {

PictureRasterizer::PictureRasterizer(PictureTable* picture_table)
  : picture_table_(picture_table) {
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
}

void PictureRasterizer::Rasterize() {
}

}  // namespace sky
