// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/backdrop_filter_layer.h"

namespace flutter {

BackdropFilterLayer::BackdropFilterLayer(sk_sp<SkImageFilter> filter)
    : filter_(std::move(filter)) {}

void BackdropFilterLayer::Preroll(PrerollContext* context,
                                  const SkMatrix& matrix) {
  size_t count = 0;
  if (context->damage_context) {
    count = context->damage_context->layer_entries_count();
  }
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context, true, bool(filter_));
  ContainerLayer::Preroll(context, matrix);
  if (context->damage_context) {
    // Adjust display bounds to layers below this layer
    context->damage_context->ApplyImageFilter(0, count, filter_.get(), matrix,
                                              paint_bounds());
    // Push entry before children
    context->damage_context->PushLayerEntry(this, compare, matrix, *context,
                                            count);
  }
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
  auto d1 = bf1->filter_->serialize();
  auto d2 = bf2->filter_->serialize();
  return d1 && d1->equals(d2.get());
}

}  // namespace flutter
