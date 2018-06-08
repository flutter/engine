// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/layer.h"

#include "flutter/flow/paint_utils.h"
#include "third_party/skia/include/core/SkColorFilter.h"

namespace flow {

Layer::Layer()
    : parent_(nullptr),
      needs_system_composite_(false),
      paint_bounds_(SkRect::MakeEmpty()) {}

Layer::~Layer() = default;

void Layer::Preroll(PrerollContext* context,
                    const SkMatrix& matrix,
                    const SkIRect& device_clip) {}

#if defined(OS_FUCHSIA)
void Layer::UpdateScene(SceneUpdateContext& context) {}
#endif  // defined(OS_FUCHSIA)

Layer::AutoSaveLayer::AutoSaveLayer(const PaintContext& paint_context,
                                    const SkRect& bounds,
                                    const SkPaint* paint)
    : paint_context_(paint_context), bounds_(bounds) {
  paint_context_.canvas.saveLayer(bounds_, paint);
}

Layer::AutoSaveLayer::AutoSaveLayer(const PaintContext& paint_context,
                                    const SkCanvas::SaveLayerRec& layer_rec)
    : paint_context_(paint_context), bounds_(*layer_rec.fBounds) {
  paint_context_.canvas.saveLayer(layer_rec);
}

Layer::AutoSaveLayer::~AutoSaveLayer() {
  if (paint_context_.checkerboard_offscreen_layers) {
    DrawCheckerboard(&paint_context_.canvas, bounds_);
  }
  paint_context_.canvas.restore();
}

SkIRect Layer::ComputeDeviceIRect(const SkMatrix& ctm, const SkRect& rect) {
  SkRect device_rect;
  ctm.mapRect(&device_rect, rect);
  SkIRect device_irect;
  device_rect.roundOut(&device_irect);
  return device_irect;
}

}  // namespace flow
