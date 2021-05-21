// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/display_list_layer.h"

#include "flutter/flow/display_list_interpreter.h"

namespace flutter {

DisplayListLayer::DisplayListLayer(
    const SkPoint& offset,
    const SkRect& cull_rect,
    const SkRect& draw_rect,
    std::shared_ptr<std::vector<uint8_t>> ops,
    std::shared_ptr<std::vector<float>> data,
    std::shared_ptr<std::vector<DisplayListRefHolder>> refs,
    bool is_complex,
    bool will_change)
    : offset_(offset),
      cull_rect_(cull_rect),
      draw_rect_(draw_rect),
      ops_vector_(ops),
      data_vector_(data),
      ref_vector_(refs),
      is_complex_(is_complex),
      will_change_(will_change) {}

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

// TODO(flar) Implement display list comparisons and ::IsReplacing method
void DisplayListLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const DisplayListLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (offset_ != prev->offset_ || cull_rect_ != prev->cull_rect_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }
  context->PushTransform(SkMatrix::Translate(offset_.x(), offset_.y()));
  SkRect bounds = draw_rect_;
  if (!bounds.intersect(cull_rect_)) {
    bounds.setEmpty();
  }
  context->AddLayerBounds(bounds);
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

void DisplayListLayer::Preroll(PrerollContext* context,
                               const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "DisplayListLayer::Preroll");

#if defined(LEGACY_FUCHSIA_EMBEDDER)
  CheckForChildLayerBelow(context);
#endif

  // TODO(flar): implement DisplayList raster caching

  SkRect bounds = draw_rect_;
  if (true || bounds.intersect(cull_rect_)) {
    bounds.offset(offset_.x(), offset_.y());
  } else {
    bounds.setEmpty();
  }
  set_paint_bounds(bounds);
}

void DisplayListLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "DisplayListLayer::Paint");
  FML_DCHECK(needs_painting(context));

  SkAutoCanvasRestore save(context.leaf_nodes_canvas, true);
  context.leaf_nodes_canvas->translate(offset_.x(), offset_.y());
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  context.leaf_nodes_canvas->setMatrix(RasterCache::GetIntegralTransCTM(
      context.leaf_nodes_canvas->getTotalMatrix()));
#endif

  // TODO(flar): implement DisplayList raster caching

  DisplayListInterpreter interpreter(ops_vector_, data_vector_, ref_vector_);
  interpreter.Rasterize(context.leaf_nodes_canvas);

  SkPaint paint;
  paint.setColor(is_complex_
                     ? (will_change_ ? SkColors::kRed : SkColors::kYellow)
                     : (will_change_ ? SkColors::kBlue : SkColors::kGreen));
}

}  // namespace flutter
