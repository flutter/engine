// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/backdrop_filter_layer.h"

namespace flutter {

BackdropFilterLayer::BackdropFilterLayer(sk_sp<SkImageFilter> filter)
    : filter_(std::move(filter)) {}

void BackdropFilterLayer::Preroll(PrerollContext* context,
                                  const SkMatrix& matrix) {
  if (context->damage_context) {
    // Backdrop filter blurs everything within clip area
    context->damage_context->AddLayerContribution(this, compare, matrix,
                                                  context->cull_rect, *context);
    context->damage_context->AddReadbackArea(
        matrix, filter_.get()->computeFastBounds(context->cull_rect));
  }

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

bool BackdropFilterLayer::compare(const Layer* l1, const Layer* l2) {
  const auto* bf1 = reinterpret_cast<const BackdropFilterLayer*>(l1);
  const auto* bf2 = reinterpret_cast<const BackdropFilterLayer*>(l2);
  return bf1->filter_ == bf2->filter_;
}

}  // namespace flutter
