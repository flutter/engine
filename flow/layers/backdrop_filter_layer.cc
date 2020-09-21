// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/backdrop_filter_layer.h"

namespace flutter {

BackdropFilterLayer::BackdropFilterLayer(sk_sp<SkImageFilter> filter)
    : filter_(std::move(filter)) {}

void BackdropFilterLayer::Diff(DiffContext* context, const Layer* old_layer) {
  auto subtree = context->BeginSubtree();
  auto* prev = reinterpret_cast<const BackdropFilterLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    assert(prev);
    if (filter_ != prev->filter_) {
      context->MarkSubtreeDirty(old_layer->paint_region());
    }
  }

  // Backdrop filter paints everywhere in cull rect
  auto paint_bounds = filter_->computeFastBounds(context->GetCullRect());
  if (context->IsSubtreeDirty()) {
    context->AddPaintRegion(paint_bounds);
  }
  context->AddReadbackRegion(paint_bounds);

  DiffChildren(context, prev);

  set_paint_region(context->CurrentSubtreeRegion());
}

void BackdropFilterLayer::Preroll(PrerollContext* context,
                                  const SkMatrix& matrix) {
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context, true, bool(filter_));
  ContainerLayer::Preroll(context, matrix);
}

void BackdropFilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "BackdropFilterLayer::Paint");
  FML_DCHECK(needs_painting());

  Layer::AutoSaveLayer save = Layer::AutoSaveLayer::Create(
      context,
      SkCanvas::SaveLayerRec{&paint_bounds(), nullptr, filter_.get(), 0});
  PaintChildren(context);
}

}  // namespace flutter
