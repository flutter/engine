// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/raster_transform_layer.h"

#include <optional>

namespace flutter {

RasterTransformLayer::RasterTransformLayer(const SkMatrix& transform,
                                           const SkV2& raster_scale,
                                           const SkSamplingOptions& sampling)
    : transform_(transform), raster_scale_(raster_scale), sampling_(sampling) {
  // Checks (in some degree) that SkMatrix transform_ is valid and initialized.
  //
  // If transform_ is uninitialized, this assert may look flaky as it doesn't
  // fail all the time, and some rerun may make it pass. But don't ignore it and
  // just rerun the test if this is triggered, since even a flaky failure here
  // may signify a potentially big problem in the code.
  //
  // We have to write this flaky test because there is no reliable way to test
  // whether a variable is initialized or not in C++.
  FML_DCHECK(transform_.isFinite());
  if (!transform_.isFinite()) {
    FML_LOG(ERROR)
        << "RasterTransformLayer is constructed with an invalid matrix.";
    transform_.setIdentity();
  }
  set_layer_can_inherit_opacity(true);
}

void RasterTransformLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const RasterTransformLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (transform_ != prev->transform_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }
  context->PushTransform(transform_);
  DiffChildren(context, prev);
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void RasterTransformLayer::Preroll(PrerollContext* context,
                                   const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "RasterTransformLayer::Preroll");

  SkMatrix raster_scale_matrix;
  raster_scale_matrix.setScale(raster_scale_.x, raster_scale_.y);

  SkMatrix child_matrix;
  child_matrix.setConcat(matrix, raster_scale_matrix);

  // Disable culling since we want to rasterize the entire child at the given
  // raster scale.
  SkRect previous_cull_rect = context->cull_rect;
  context->cull_rect = kGiantRect;

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, child_matrix, &child_paint_bounds);

  if (context->raster_cache) {
    TryToPrepareRasterCache(context, GetCacheableChild(), child_matrix);
  }

  context->cull_rect = previous_cull_rect;

  transform_.mapRect(&child_paint_bounds);
  set_paint_bounds(child_paint_bounds);
}

void RasterTransformLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "RasterTransformLayer::Paint");
  FML_DCHECK(needs_painting(context));

  if (context.raster_cache) {
    AutoCachePaint cache_paint(context);

    SkMatrix raster_scale_matrix;
    raster_scale_matrix.setScale(raster_scale_.x, raster_scale_.y);

    // Match the total matrix (ctm) with child_matrix during Preroll.
    SkAutoCanvasRestore save(context.internal_nodes_canvas, true);
    context.internal_nodes_canvas->concat(raster_scale_matrix);

    // Then scale the transform matrix to restore the original raster scale.
    SkMatrix post_raster_transform = transform_;
    post_raster_transform.postScale(1.0 / raster_scale_.x,
                                    1.0 / raster_scale_.y);

    if (context.raster_cache->DrawTransformed(
            GetCacheableChild(), *context.internal_nodes_canvas,
            post_raster_transform, sampling_, cache_paint.paint())) {
      return;
    }
  }

  SkAutoCanvasRestore save(context.internal_nodes_canvas, true);
  context.internal_nodes_canvas->concat(transform_);

  PaintChildren(context);
}

}  // namespace flutter
