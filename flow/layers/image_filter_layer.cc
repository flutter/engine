// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/image_filter_layer.h"
#include "flutter/display_list/display_list_comparable.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

ImageFilterLayer::ImageFilterLayer(std::shared_ptr<const DlImageFilter> filter)
    : CacheableContainerLayer(
          RasterCacheUtil::kMinimumRendersBeforeCachingFilterLayer),
      filter_(std::move(filter)),
      transformed_filter_(nullptr) {}

void ImageFilterLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const ImageFilterLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (NotEquals(filter_, prev->filter_)) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }

  if (filter_) {
    auto filter = filter_->makeWithLocalMatrix(context->GetTransform());
    if (filter) {
      // This transform will be applied to every child rect in the subtree
      context->PushFilterBoundsAdjustment([filter](SkRect rect) {
        SkIRect filter_out_bounds;
        filter->map_device_bounds(rect.roundOut(), SkMatrix::I(),
                                  filter_out_bounds);
        return SkRect::Make(filter_out_bounds);
      });
    }
  }
  DiffChildren(context, prev);
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void ImageFilterLayer::Preroll(PrerollContext* context,
                               const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "ImageFilterLayer::Preroll");

  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);

  AutoCache cache = AutoCache(layer_raster_cache_item_.get(), context, matrix);

  SkRect child_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_bounds);

  if (!filter_) {
    set_paint_bounds(child_bounds);
    return;
  }

  // We always paint with a saveLayer (or a cached rendering),
  // so we can always apply opacity in any of those cases.
  context->renderable_state_flags = LayerStateStack::CALLER_CAN_APPLY_OPACITY;

  const SkIRect filter_in_bounds = child_bounds.roundOut();
  SkIRect filter_out_bounds;
  filter_->map_device_bounds(filter_in_bounds, SkMatrix::I(),
                             filter_out_bounds);
  child_bounds = SkRect::Make(filter_out_bounds);

  set_paint_bounds(child_bounds);

  // CacheChildren only when the transformed_filter_ doesn't equal null.
  // So in here we reset the LayerRasterCacheItem cache state.
  layer_raster_cache_item_->MarkNotCacheChildren();

  transformed_filter_ = filter_->makeWithLocalMatrix(matrix);
  if (transformed_filter_) {
    layer_raster_cache_item_->MarkCacheChildren();
  }
}

void ImageFilterLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ImageFilterLayer::Paint");
  FML_DCHECK(needs_painting(context));

  // AutoCachePaint cache_paint(context);
  // if (context.raster_cache) {
  //   if (layer_raster_cache_item_->IsCacheChildren()) {
  //     cache_paint.setImageFilter(transformed_filter_.get());
  //   }
  //   if (layer_raster_cache_item_->Draw(context, cache_paint.sk_paint())) {
  //     return;
  //   }
  // }

  auto mutator = context.state_stack.save();
  mutator.applyImageFilter(child_paint_bounds(), filter_);

  PaintChildren(context);
}

}  // namespace flutter
