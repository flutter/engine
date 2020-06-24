// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/image_filter_layer.h"

namespace flutter {

ImageFilterLayer::ImageFilterLayer(sk_sp<SkImageFilter> filter)
    : filter_(std::move(filter)),
      transformed_filter_(nullptr),
      render_count_(1) {}

void ImageFilterLayer::Preroll(PrerollContext* context,
                               const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "ImageFilterLayer::Preroll");

  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);

  SkRect child_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_bounds);
  if (filter_) {
    const SkIRect filter_input_bounds = child_bounds.roundOut();
    SkIRect filter_output_bounds =
        filter_->filterBounds(filter_input_bounds, SkMatrix::I(),
                              SkImageFilter::kForward_MapDirection);
    child_bounds = SkRect::Make(filter_output_bounds);
  }
  set_paint_bounds(child_bounds);

  transformed_filter_ = nullptr;
  if (render_count_ >= kMinimumRendersBeforeCachingFilterLayer) {
    // We have rendered this same ImageFilterLayer object enough
    // times to consider its properties and children to be stable
    // from frame to frame so we try to cache the layer itself
    // for maximum performance.
    TryToPrepareRasterCache(context, this, matrix);
  } else if ((transformed_filter_ = filter_->makeWithLocalMatrix(matrix))) {
    // This ImageFilterLayer is not yet considered stable so we
    // increment the count to measure how many times it has been
    // seen from frame to frame.
    render_count_++;
    // And for now we will still try to cache the children as that
    // provides a large performance benefit to avoid rendering the
    // child layers themselves and also avoiding a rendering surface
    // switch to do so during the Paint phase. This benefit is seen
    // most during animations involving the ImageFilter.
    TryToPrepareRasterCache(context, GetCacheableChild(), matrix);
  }
}

void ImageFilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ImageFilterLayer::Paint");
  FML_DCHECK(needs_painting());

  if (context.raster_cache) {
    if (context.raster_cache->Draw(this, *context.leaf_nodes_canvas)) {
      return;
    }
    if (transformed_filter_) {
      SkPaint paint;
      paint.setImageFilter(transformed_filter_);

      if (context.raster_cache->Draw(GetCacheableChild(),
                                     *context.leaf_nodes_canvas, &paint)) {
        return;
      }
    }
  }

  SkPaint paint;
  paint.setImageFilter(filter_);

  // Normally a save_layer is sized to the current layer bounds, but in this
  // case the bounds of the child may not be the same as the filtered version
  // so we use the bounds of the child container which do not include any
  // modifications that the filter might apply.
  Layer::AutoSaveLayer save_layer = Layer::AutoSaveLayer::Create(
      context, GetChildContainer()->paint_bounds(), &paint);
  PaintChildren(context);
}

}  // namespace flutter
