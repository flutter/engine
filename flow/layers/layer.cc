// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer.h"

#include "flutter/flow/paint_utils.h"
#include "third_party/skia/include/core/SkColorFilter.h"

namespace flutter {

Layer::Layer()
    : paint_bounds_(SkRect::MakeEmpty()),
      unique_id_(NextUniqueID()),
      needs_system_composite_(false) {}

Layer::~Layer() = default;

uint64_t Layer::NextUniqueID() {
  static std::atomic<uint64_t> nextID(1);
  uint64_t id;
  do {
    id = nextID.fetch_add(1);
  } while (id == 0);  // 0 is reserved for an invalid id.
  return id;
}

void Layer::Preroll(PrerollContext* context, const SkMatrix& matrix) {}

Layer::AutoPrerollSaveLayer::AutoPrerollSaveLayer(
    PrerollContext* preroll_context,
    bool save_layer_is_active,
    bool layer_itself_performs_readback)
    : preroll_context_(preroll_context),
      save_layer_is_active_(save_layer_is_active),
      layer_itself_performs_readback_(layer_itself_performs_readback) {
  if (save_layer_is_active_) {
    prev_subtree_performs_readback_operation_ =
        preroll_context_->subtree_performs_readback_operation;
    preroll_context_->subtree_performs_readback_operation = false;
  }
}

Layer::AutoPrerollSaveLayer Layer::AutoPrerollSaveLayer::Create(
    PrerollContext* preroll_context,
    bool save_layer_is_active,
    bool layer_itself_performs_readback) {
  return Layer::AutoPrerollSaveLayer(preroll_context, save_layer_is_active,
                                     layer_itself_performs_readback);
}

Layer::AutoPrerollSaveLayer::~AutoPrerollSaveLayer() {
  if (save_layer_is_active_) {
    preroll_context_->subtree_performs_readback_operation =
        (prev_subtree_performs_readback_operation_ ||
         layer_itself_performs_readback_);
  }
}

#if defined(OS_FUCHSIA)
void Layer::UpdateScene(SceneUpdateContext& context) {}
#endif  // defined(OS_FUCHSIA)

Layer::AutoSaveLayer::AutoSaveLayer(const PaintContext& paint_context,
                                    const SkRect& bounds,
                                    const SkPaint* paint)
    : paint_context_(paint_context), bounds_(bounds) {
  paint_context_.internal_nodes_canvas->saveLayer(bounds_, paint);
}

Layer::AutoSaveLayer::AutoSaveLayer(const PaintContext& paint_context,
                                    const SkCanvas::SaveLayerRec& layer_rec)
    : paint_context_(paint_context), bounds_(*layer_rec.fBounds) {
  paint_context_.internal_nodes_canvas->saveLayer(layer_rec);
}

Layer::AutoSaveLayer Layer::AutoSaveLayer::Create(
    const PaintContext& paint_context,
    const SkRect& bounds,
    const SkPaint* paint) {
  return Layer::AutoSaveLayer(paint_context, bounds, paint);
}

Layer::AutoSaveLayer Layer::AutoSaveLayer::Create(
    const PaintContext& paint_context,
    const SkCanvas::SaveLayerRec& layer_rec) {
  return Layer::AutoSaveLayer(paint_context, layer_rec);
}

Layer::AutoSaveLayer::~AutoSaveLayer() {
  if (paint_context_.checkerboard_offscreen_layers) {
    DrawCheckerboard(paint_context_.internal_nodes_canvas, bounds_);
  }
  paint_context_.internal_nodes_canvas->restore();
}

}  // namespace flutter
