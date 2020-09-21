// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/color_filter_layer.h"

namespace flutter {

ColorFilterLayer::ColorFilterLayer(sk_sp<SkColorFilter> filter)
    : filter_(std::move(filter)) {}

void ColorFilterLayer::Diff(DiffContext* context, const Layer* old_layer) {
  auto subtree = context->BeginSubtree();
  auto* prev = reinterpret_cast<const ColorFilterLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    assert(prev);
    if (filter_ != prev->filter_) {
      context->MarkSubtreeDirty(old_layer->paint_region());
    }
  }

  DiffChildren(context, prev);

  set_paint_region(context->CurrentSubtreeRegion());
}

void ColorFilterLayer::Preroll(PrerollContext* context,
                               const SkMatrix& matrix) {
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);
  ContainerLayer::Preroll(context, matrix);
}

void ColorFilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ColorFilterLayer::Paint");
  FML_DCHECK(needs_painting());

  SkPaint paint;
  paint.setColorFilter(filter_);

  Layer::AutoSaveLayer save =
      Layer::AutoSaveLayer::Create(context, paint_bounds(), &paint);
  PaintChildren(context);
}

}  // namespace flutter
