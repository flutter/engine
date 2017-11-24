// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/hole_layer.h"

namespace flow {

HoleLayer::HoleLayer() = default;

HoleLayer::~HoleLayer() = default;

void HoleLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  set_paint_bounds(SkRect::MakeXYWH(offset_.x(), offset_.y(), size_.width(),
                                    size_.height()));
}

void HoleLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "HoleLayer::Paint");
  FXL_DCHECK(needs_painting());
  SkPaint transparent;
  transparent.setBlendMode(SkBlendMode::kSrc);
  transparent.setColor(0);
  context.canvas.drawRect(paint_bounds(), transparent);
}

}  // namespace flow
